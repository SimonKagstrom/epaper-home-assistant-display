#include "monochromer.h"
#include "epaper_display.h"
#include "ir_sensor.h"
#include "filesystem_monitor.h"

#include "utils.hh"

#include <optional>
#include <semaphore>
#include <map>

#include <stdio.h>

#include <unistd.h>

using namespace std::chrono_literals;

namespace
{

class Converter
{
public:
    explicit Converter(const std::filesystem::path &directory, const std::vector<std::string> &conversions) :
        m_monitor(IFilesystemMonitor::create(directory)),
        m_conversions(conversions)
    {
    }

    std::vector<std::filesystem::path> updateFiles()
    {
        auto out = std::vector<std::filesystem::path>();

        for (auto &cur : m_monitor->getUpdatedFiles())
        {
            m_processor[cur] = IMonochromer::create(cur, m_conversions);
            m_imageData[cur] = m_processor[cur]->process();
        }

        for (const auto &[k,v] : m_imageData)
        {
            out.push_back(k);
        }

        return out;
    }

    std::optional<std::span<uint8_t>> getImage(const std::filesystem::path &image)
    {
        if (m_imageData.contains(image))
        {
            return m_imageData[image];
        }
        return {};
    }

private:
    std::unique_ptr<IFilesystemMonitor> m_monitor;
    std::map<std::filesystem::path, std::unique_ptr<IMonochromer>> m_processor;
    std::map<std::filesystem::path, std::span<uint8_t>> m_imageData;

    std::vector<std::string> m_conversions;
};

class Main
{
public:
    explicit Main(const std::filesystem::path &directory, const std::vector<std::string> &conversions) :
        m_converter(std::make_unique<Converter>(directory, conversions))
    {
        m_display->clear();

        m_motionCookie = m_motionSensor->listenToMotion([this]()
        {
            m_motionSemaphore.release();
        });
    }

    void run()
    {
        m_files = m_converter->updateFiles();

        m_current = 0;
        bool motionChange = false;
        auto before = 100u;
        auto sleepAfterDraw = true;
        while (true)
        {
            auto delta = 0u;
            if (motionChange && m_motionSensor->hasMotion())
            {
                delta = 1;
                sleepAfterDraw = false;
            }
            else if (!motionChange) // timeout
            {
                m_files = m_converter->updateFiles();

                // Skip to the next image
                delta = 1;
                sleepAfterDraw = true;
            }

            m_current = (m_current + delta) % m_files.size();

            if (m_current != before)
            {
                if (auto img = m_converter->getImage(m_files[m_current]))
                {
                    m_display->drawImage(*img);
                }

                if (sleepAfterDraw)
                {
                    m_display->sleep();
                }
            }

            motionChange = m_motionSemaphore.try_acquire_for(10min);
            before = m_current;
        }
    }

private:
    std::unique_ptr<IEpaperDisplay> m_display{IEpaperDisplay::create()};
    std::unique_ptr<Converter> m_converter;
    std::unique_ptr<IIRSensor> m_motionSensor{IIRSensor::create()};
    std::binary_semaphore m_motionSemaphore{0};
    std::unique_ptr<IIRSensor::ICookie> m_motionCookie;

    std::vector<std::filesystem::path> m_files;
    unsigned m_current{0};
};

}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: dir\n");
        return 1;
    }

    std::string dir(argv[1]);
    std::vector<std::string> conversions;

    for (auto i = 2u; i < argc; i++)
    {
        auto parts = split_string(argv[i], " ");

        for (auto &cur : parts)
        {
            conversions.emplace_back(cur);
        }
    }

    auto m = std::make_unique<Main>(dir, conversions);

    m->run();

    return 0;
}

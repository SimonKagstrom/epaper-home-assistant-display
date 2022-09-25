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
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

namespace
{

class Converter
{
public:
    using IndexToRaw = std::map<unsigned, std::vector<uint8_t>>;

    explicit Converter(const std::filesystem::path &directory, const std::vector<std::string> &conversions) :
        m_monitor(IFilesystemMonitor::create(directory)),
        m_conversions(conversions)
    {
        // Get an initial update
        updateFiles();

        m_updateThread = std::thread([this]()
        {
            while (true)
            {
                std::this_thread::sleep_for(1min);
                updateFiles();
            }
        });
    }

    const std::shared_ptr<IndexToRaw> getImages()
    {
        std::lock_guard lock(m_mutex);

        return m_currentImages;
    }

private:
    void updateFiles()
    {
        std::map<std::filesystem::path, std::span<uint8_t>> updatedImageData;

        auto updated = m_monitor->getUpdatedFiles();
        if (updated.empty())
        {
            return;
        }

        for (auto &cur : updated)
        {
            m_processor[cur] = IMonochromer::create(cur, m_conversions);
            updatedImageData[cur] = m_processor[cur]->process();
        }

        printf("New images, %zu updated\n", updated.size());
        std::lock_guard lock(m_mutex);
        auto newData = *m_currentImages;

        unsigned i = 0;
        for (const auto &[k,v] : updatedImageData)
        {
            newData.emplace(i++, std::vector<uint8_t>(v.begin(), v.end()));
        }

        // Create a new set, which gets used the next time getImages() is called
        m_currentImages = nullptr;
        m_currentImages = std::make_shared<IndexToRaw>(newData);
    }

    std::thread m_updateThread;
    std::unique_ptr<IFilesystemMonitor> m_monitor;
    std::map<std::filesystem::path, std::unique_ptr<IMonochromer>> m_processor;
    std::map<std::filesystem::path, std::span<uint8_t>> m_imageData;

    std::shared_ptr<IndexToRaw> m_currentImages{std::make_shared<IndexToRaw>()};

    std::vector<std::string> m_conversions;
    std::mutex m_mutex;
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
        m_current = 0;
        bool motionChange = false;
        auto sleepAfterDraw = true;
        while (true)
        {
            auto images = m_converter->getImages();

            auto delta = 0u;
            if (motionChange && m_motionSensor->hasMotion())
            {
                delta = 1;
                sleepAfterDraw = false;
            }
            else if (!motionChange) // timeout
            {
                // Skip to the next image
                delta = 1;
                sleepAfterDraw = true;
            }

            m_current = (m_current + delta) % images->size();

            if (images->contains(m_current))
            {
                m_display->drawImage(images->at(m_current));
            }

            if (sleepAfterDraw)
            {
                m_display->sleep();
            }
            else
            {
                motionChange = m_motionSemaphore.try_acquire_for(10s);
                if (motionChange && m_motionSensor->hasMotion())
                {
                    // Movement, so go to next
                    continue;
                }

                // Otherwise sleep
                m_display->sleep();
            }

            motionChange = m_motionSemaphore.try_acquire_for(10min);
        }
    }

private:
    std::unique_ptr<IEpaperDisplay> m_display{IEpaperDisplay::create()};
    std::unique_ptr<Converter> m_converter;
    std::unique_ptr<IIRSensor> m_motionSensor{IIRSensor::create()};
    std::binary_semaphore m_motionSemaphore{0};
    std::unique_ptr<IIRSensor::ICookie> m_motionCookie;

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

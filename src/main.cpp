#include "monochromer.h"
#include "epaper_display.h"
#include "ir_sensor.h"
#include "converter.h"

#include "utils.hh"

#include <optional>
#include <semaphore>
#include <map>

#include <stdio.h>

#include <unistd.h>

using namespace std::chrono_literals;

namespace
{

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
        unsigned current = 0;
        bool motionChange = false;
        auto sleepAfterDraw = true;
        while (true)
        {
            if (motionChange && m_motionSensor->hasMotion())
            {
                unsigned next = (current + 1) % m_files.size();

                // Skip to the next image
                sleepAfterDraw = false;
                current = next;
            }
            else if (!motionChange) // timeout
            {
                // Display the first image
                current = 0;
                sleepAfterDraw = true;
            }

            if (current == 0)
            {
                printf("Updating image data\n");
                m_files = m_converter->updateFiles();
            }


            printf("Drawing image %u\n", current);
            if (auto img = m_converter->getImage(m_files[current]))
            {
                m_display->drawImage(*img);
            }
            else
            {
                printf("No image %u?\n", current);
                continue;
            }

            m_display->flip();

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

    std::vector<std::filesystem::path> m_files;
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

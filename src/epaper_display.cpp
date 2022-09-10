#include "epaper_display.h"

extern "C"
{
#include <EPD_7in5_V2.h>
}

namespace
{

class EpaperDisplay : public IEpaperDisplay
{
public:
    EpaperDisplay()
    {
        if (DEV_Module_Init() != 0)
        {
            exit(1);
        }
        EPD_7IN5_V2_Init();
    }

    ~EpaperDisplay() final
    {
        sleep();
    }

    EpaperDisplay(const EpaperDisplay &) = delete;
    EpaperDisplay operator=(EpaperDisplay&) = delete;

    void clear() final
    {
        awake();
        EPD_7IN5_V2_Clear();
    }

    void drawImage(std::span<uint8_t> data) final
    {
        awake();
        EPD_7IN5_V2_Display(data.data());
    }

    void sleep() final
    {
        EPD_7IN5_V2_Sleep();
        m_asleep = true;
    }

private:
    void awake()
    {
        if (m_asleep)
        {
            EPD_7IN5_V2_Init();
            m_asleep = false;
        }
    }

    bool m_asleep{false};
};

}

std::unique_ptr<IEpaperDisplay> IEpaperDisplay::create()
{
    return std::make_unique<EpaperDisplay>();
}

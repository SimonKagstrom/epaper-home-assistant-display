#include "epaper_display.h"

extern "C"
{
#include <EPD_7in5_V2.h>
}

namespace
{

void SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

void SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

void WaitUntilIdle(void)
{
	do{
		DEV_Delay_ms(1);
	}while(!(DEV_Digital_Read(EPD_BUSY_PIN)));
	DEV_Delay_ms(1);
}


void WriteNEWImage(const UBYTE *blackimage)
{
    UDOUBLE Width, Height;
    Width =(EPD_7IN5_V2_WIDTH % 8 == 0)?(EPD_7IN5_V2_WIDTH / 8 ):(EPD_7IN5_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5_V2_HEIGHT;

    WaitUntilIdle();

    SendCommand(0x13);
    for (UDOUBLE j = 0; j < Height; j++) {
        for (UDOUBLE i = 0; i < Width; i++) {
            SendData(~blackimage[i + j * Width]);
        }
    }
}

void TurnOnDisplay(void)
{
    WaitUntilIdle();
    SendCommand(0x12);			//DISPLAY REFRESH
    DEV_Delay_ms(100);	        //!!!The delay here is necessary, 200uS at least!!!
}


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
        printf("Clearing display\n");
        awake();
        EPD_7IN5_V2_Clear();
    }

    void drawImage(std::span<uint8_t> data) final
    {
        printf("Draw\n");
        awake();
        WriteNEWImage(data.data());
//        EPD_7IN5_V2_Display(data.data());
    }

    void flip() final
    {
        printf("Flip\n");
        TurnOnDisplay();
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

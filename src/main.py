#!/usr/bin/env python3

from waveshare_epd import epd7in5_V2 as display
import logging
import sys
import os
import time
from PIL import Image

if __name__ == "__main__":
    try:
        if len(sys.argv) != 2:
            print("Arg: image to display")
            sys.exit(1)

        print("STARTING")
        epd = display.EPD()

        epd.init()
        epd.Clear()

        print("main loop")
        while True:
            Himage = Image.open(sys.argv[1])

            epd.display(epd.getbuffer(Himage))

            time.sleep(10)

            epd.Clear()
            display.epdconfig.module_exit()
            sys.exit(1)

    except IOError as e:
        logging.info(e)

    except KeyboardInterrupt:
        logging.info("ctrl + c:")
        display.epdconfig.module_exit()
        sys.exit(1)

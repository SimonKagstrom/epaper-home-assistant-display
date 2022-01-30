#!/usr/bin/env python3

import shutil
from waveshare_epd import epd7in5_V2 as display
import logging
import sys
import os
from pathlib import Path
import time
from PIL import Image

import monochromer

if __name__ == "__main__":
    try:
        if len(sys.argv) != 3:
            print("Usage: XXX <path> <conversions>")
            sys.exit(1)

        path = sys.argv[1]
        conversions = sys.argv[2].split()
        while not Path("{}/0.png".format(path)).is_file:
            print("Need 0.png (and possible 1...X.png) in {}. Waiting...".format(path))
            time.sleep(1)

        epd = display.EPD()

        epd.init()
        epd.Clear()
        powered = True

        while True:
            images = []

            for i in range(0, 10):
                imageName = "{}/{}.png".format(path, i)
                if os.path.exists(imageName):
                    m = monochromer.Monochromer(imageName, conversions)
                    images.append(m.process())

            if len(images) == 0:
                print("0.png removed??")
                sys.exit(1)

            for img in images:
                epd.display(epd.getbuffer(img))
                time.sleep(10)

            # This is the default display
            epd.display(epd.getbuffer(images[0]))

            powered = False
            epd.sleep()
            for i in range(0, 5 * 60):
                # check sensor here for next image display
                if False:
                    pass
                time.sleep(1)

            if not powered:
                powered = True
                epd.init()

    except IOError as e:
        logging.info(e)

    except KeyboardInterrupt:
        logging.info("ctrl + c:")
        if not powered:
            epd.init()
            epd.Clear()
        display.epdconfig.module_exit()
        sys.exit(1)

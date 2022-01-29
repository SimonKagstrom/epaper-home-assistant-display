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

        cur_image = 0

        while True:
            imageName = "{}/{}.png".format(path, cur_image)
            if not Path(imageName).is_file:
                cur_image = 0
                if not Path("{}/0.png".format(path)).is_file:
                    print("0.png removed??")
                    sys.exit(1)
                continue

            m = monochromer.Monochromer(imageName, conversions)
            Himage = m.process()

            epd.display(epd.getbuffer(Himage))

            powered = True
            for i in range(0, 5 * 60):
                # Power down after 10 seconds
                if powered and i == 10:
                    epd.sleep()
                    powered = False
                # check sensor here for next image display
                if False:
                    cur_image = cur_image + 1
                time.sleep(1)

            if not powered:
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

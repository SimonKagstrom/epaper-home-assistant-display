#!/usr/bin/env python3

import shutil
from waveshare_epd import epd7in5_V2 as display
import logging
import sys
import os
from pathlib import Path
import time
from PIL import Image
from RPi import GPIO

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

        MOTION_PIN = 26
        GPIO.setup(MOTION_PIN, GPIO.IN)

        cur_image = 0

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

            # This is the default display
            epd.display(epd.getbuffer(images[cur_image]))
            cur_image = cur_image + 1

            powered = False
            epd.sleep()
            for i in range(0, 5 * 60):

                if GPIO.input(MOTION_PIN):
                    if cur_image >= len(images):
                        cur_image = 0
                    if not powered:
                        powered = True
                        epd.init()
                    epd.display(epd.getbuffer(images[cur_image]))
                    cur_image = cur_image + 1
                    powered = False
                    epd.sleep()
                    time.sleep(10)
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

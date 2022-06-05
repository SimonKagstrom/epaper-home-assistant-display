#/usr/bin/env python3

from waveshare_epd import epd7in5_V2 as display
from PIL import Image
from RPi import GPIO

import threading
import pathlib
import logging
import os
import sys
import queue

import time
import monochromer

MOTION_PIN = 26

class StatCache:
    def __init__(self, files : list):
        self.stat_cache = {}
        self.files = files

    def updated(self):
        updated = False

        # Check for removed files
        for f in self.stat_cache.keys():
            if not pathlib.Path(f).is_file:
                del self.stat_cache[f]
                updated = True

        for f in self.files:
            if not os.path.exists(f) or not pathlib.Path(f).is_file:
                continue

            s = os.lstat(f)
            if f in self.stat_cache:
                updated = updated | (self.stat_cache[f] != s.st_mtime)
            else:
                updated = True
            self.stat_cache[f] = s.st_mtime

        return updated

    def present_files(self):
        return self.stat_cache.keys()

def file_conversion(path : str, conversions: list, out_queue : queue.Queue):
    image_names = []

    for i in range(0, 20):
        image_names.append(os.path.join(path,"{}.png".format(i)))

    stat_cache = StatCache(image_names)

    while True:
        while not stat_cache.updated():
            time.sleep(5)
        images = []
        files = stat_cache.present_files()

        for f in files:
            m = monochromer.Monochromer(f, conversions)
            images.append(m.process())

        logging.info("converted {}".format(files))
        out_queue.put(images)


def wait_for_motion(out_queue : queue.Queue):
    while True:
        if GPIO.input(MOTION_PIN):
            out_queue.put(True)
        time.sleep(0.25)


def display_image(epd, image : Image):
    epd.display(epd.getbuffer(image))

def power_down(epd : display.EPD):
    logging.info("power down")
    epd.sleep()
    epd.is_powered = False

def power_up(epd : display.EPD):
    if not epd.is_powered:
        logging.info("power up")
        epd.init()
    epd.is_powered = True


def display_loop(epd : display.EPD, file_queue : queue.Queue, motion_queue : queue.Queue):

    # Wait for an initial set of converted images
    converted = file_queue.get()
    cur = 0
    change_timestamp = time.process_time()

    display_image(epd, converted[cur])

    # Clear the queue for starters
    try:
        motion_queue.get(block=False, timeout=1)
    except:
        pass

    while True:
        motion = False
        try:
            motion = motion_queue.get(block=False, timeout=1)
        except TimeoutError:
            pass
        except queue.Empty:
            pass

        try:
            converted = file_queue.get(block=False)
        except TimeoutError:
            pass
        except queue.Empty:
            pass

        time_diff = time.process_time() - change_timestamp

        if not motion and time_diff > 20 and epd.is_powered:
            power_down(epd)

        if not motion and time_diff < 5 * 60:
            continue

        if motion and time_diff < 1:
            # Don't change too often
            continue

        cur = cur + 1
        if cur >= len(converted):
            cur = 0

        # Change to the next image and display
        power_up(epd)
        logging.info("display {}, with motion {} and time diff {}".format(cur, motion, time_diff))
        display_image(epd, converted[cur])
        change_timestamp = time.process_time()


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: XXX <path> <conversions>")
        sys.exit(1)

    epd = display.EPD()
    epd.is_powered = True

    epd.init()
    epd.Clear()

    GPIO.setup(MOTION_PIN, GPIO.IN)

    logging.getLogger().setLevel(logging.INFO)

    path = sys.argv[1]
    conversions = sys.argv[2].split()

    motion_queue = queue.Queue(1)
    file_queue = queue.Queue(1)

    motion_process = threading.Thread(target=wait_for_motion, args=(motion_queue,))
    file_conversion_process = threading.Thread(target=file_conversion, args=(path, conversions, file_queue))
    display_process = threading.Thread(target=display_loop, args=(epd, file_queue, motion_queue))

    try:
        motion_process.start()
        file_conversion_process.start()
        display_process.start()

    except IOError as e:
        logging.info(e)

    except KeyboardInterrupt:
        logging.info("ctrl + c:")
        if not epd.is_powered:
            epd.init()
            epd.Clear()
        display.epdconfig.module_exit()
        sys.exit(1)

    motion_process.join()
    file_conversion_process.join()
    display_process.join()

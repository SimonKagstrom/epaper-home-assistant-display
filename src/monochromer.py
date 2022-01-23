#!/usr/bin/env python3

import sys
from PIL import Image

class Conversion:
    def __init__(self, image, color, type):
        self.color = int("0x{}".format(color[0:2]), base=16), int("0x{}".format(color[2:4]), base=16), int("0x{}".format(color[4:6]), base=16)
        self.type = type

        self.width = image.width
        self.height = image.height

        if type == "horizontal":
            self.convertPixel = self._convertHorizontal
        elif type == "vertical":
            self.convertPixel = self._convertVertical
        elif type == "dotted":
            self.convertPixel = self._convertDotted
        elif type == "rightslant":
            self.convertPixel = self._convertRightSlant
        elif type == "leftslant":
            self.convertPixel = self._convertLeftlant

    def _convertVertical(self, coordinates):
        x,y = coordinates
        r,g,b = (0,0,0)

        if x % 6 < 5:
            r,g,b = (0xff,0xff,0xff)

        return (r,g,b)

    def _convertHorizontal(self, coordinates):
        x,y = coordinates
        r,g,b = (0,0,0)


        if y % 6 < 5:
            r,g,b = (0xff,0xff,0xff)

        return (r,g,b)

    def _convertDotted(self, coordinates):
        x,y = coordinates
        r,g,b = (0xff,0xff,0xff)

        if y % 8 == 0 and x % 8 == 0:
            r,g,b = (0,0,0)

        return (r,g,b)

    def _convertRightSlant(self, coordinates):
        x,y = coordinates
        r,g,b = (0,0,0)

        if (x+y) % 6 < 5:
            r,g,b = (0xff,0xff,0xff)

        return (r,g,b)

    def _convertLeftlant(self, coordinates):
        x,y = coordinates
        r,g,b = (0,0,0)

        if (x-y) % 6 < 5:
            r,g,b = (0xff,0xff,0xff)

        return (r,g,b)

class Monochromer:
    def __init__(self, filename, conversions):
        self.conversions = {}

        self.image = Image.open(filename)
        for x in conversions:
            cur = self._parseConversion(x)
            self.conversions[cur.color] = cur

    def _parseConversion(self, x):
        parts=x.split("=")
        if len(parts) != 2:
            raise Exception("Need color=type")
        color = parts[0]
        type = parts[1]
        if type not in ["horizontal", "vertical", "leftslant", "rightslant", "dotted"]:
            raise Exception("type {} is wrong".format(type))

        return Conversion(self.image, color, type)

    def process(self):
        rgb_im = self.image

        for x in range(0, self.image.width):
            for y in range(0, self.image.height):
                r,g,b, a = rgb_im.getpixel((x,y))
                if (r,g,b) in self.conversions:
                    cur = self.conversions[(r,g,b)]
                    rgb_im.putpixel((x,y), cur.convertPixel((x,y)))

        self.image.save("kalle.png")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(
            "Usage: XXX <filename> [conversions]\n"
            "Where conversions are lists of\n"
            "   RRGGBB=type\n\n"
            "Colors are given in hex, type is one of\n"
            "horizontal, vertical, leftslant, rightslant, dotted\n")
        sys.exit(1)

    m = Monochromer(sys.argv[1], sys.argv[2:])
    m.process()

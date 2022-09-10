FROM balenalib/raspberry-pi-alpine:3.16

RUN apk --no-cache upgrade
RUN apk add --no-cache python3 py3-pip wiringpi fmt

ENV BUILD_DEPS="build-base \
                cmake \
                g++ \
                python3-dev \
                jpeg-dev \
                libpng-dev \
                wiringpi-dev \
                fmt-dev \
"

RUN apk add --no-cache zlib-dev
RUN apk add --no-cache --virtual .build-deps ${BUILD_DEPS}

ENV LIBRARY_PATH=/lib:/usr/lib
RUN python3 -m pip install --upgrade pip
RUN pip3 install pillow spidev RPi.GPIO

RUN wget -O /tmp/CImg_latest.zip http://cimg.eu/files/CImg_latest.zip && cd /tmp/ && unzip CImg_latest.zip && cp CImg-*/CImg.h /usr/include/

RUN echo "Raspbian          " > /etc/issue

#RUN apk del ${BUILD_DEPS}

COPY waveshare-lib/RaspberryPi_JetsonNano/python/lib/ /usr/lib/python3.8
COPY src/* /app/

CMD python3 /app/main.py ${SRC_DIR} "${CONVERSIONS}"

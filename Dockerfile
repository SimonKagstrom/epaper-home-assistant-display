FROM balenalib/raspberry-pi-alpine:3.12

RUN apk --no-cache upgrade
RUN apk add --no-cache python3 py3-pip

ENV BUILD_DEPS="build-base \
                python3-dev \
                jpeg-dev \
                libpng-dev \
"

RUN apk add --no-cache zlib-dev
RUN apk add --no-cache --virtual .build-deps ${BUILD_DEPS}

ENV LIBRARY_PATH=/lib:/usr/lib
RUN python3 -m pip install --upgrade pip
RUN pip3 install pillow spidev RPi.GPIO

#RUN apk del ${BUILD_DEPS}

COPY waveshare-lib/RaspberryPi_JetsonNano/python/lib/ /usr/lib/python3.8
COPY src/* /app/

CMD python3 /app/main.py ${SRC_DIR} "${CONVERSIONS}"

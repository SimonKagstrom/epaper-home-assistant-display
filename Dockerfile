FROM balenalib/raspberry-pi-alpine:3.18

RUN apk --no-cache upgrade
RUN apk add --no-cache python3 py3-pip wiringpi fmt

ENV BUILD_DEPS="build-base \
                cmake \
                make \
                g++ \
                libgpiod-dev \
                python3-dev \
                jpeg-dev \
                libpng-dev \
                wiringpi-dev \
                fmt-dev \
"

RUN apk add --no-cache zlib-dev
RUN apk add --no-cache libpng
RUN apk add --no-cache ${BUILD_DEPS}

ENV LIBRARY_PATH=/lib:/usr/lib
#RUN python3 -m pip install --upgrade pip
#RUN pip3 install pillow spidev RPi.GPIO

RUN wget -O /tmp/CImg_latest.zip http://cimg.eu/files/CImg_latest.zip && cd /tmp/ && unzip CImg_latest.zip && cp CImg-*/CImg.h /usr/include/

# For the waveshare library
RUN echo "Raspbian          " > /etc/issue

ADD . /src
RUN cmake -B /tmp/build -DCMAKE_BUILD_TYPE=Release /src && cd /tmp/build && make -j2 && strip /tmp/build/epaper-display
RUN cp /tmp/build/epaper-display /usr/bin

RUN apk del ${BUILD_DEPS}
RUN rm -rf /src /tmp/build /usr/include/

CMD /usr/bin/epaper-display ${SRC_DIR} "${CONVERSIONS}"

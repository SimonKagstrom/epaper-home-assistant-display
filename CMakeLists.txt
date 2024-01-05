cmake_minimum_required (VERSION 3.16)
project (epaper-home-assistant-display)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(epd
    waveshare-lib/RaspberryPi_JetsonNano/c/lib/Config/DEV_Config.c
#    waveshare-lib/RaspberryPi_JetsonNano/c/lib/Config/RPI_sysfs_gpio.c
    waveshare-lib/RaspberryPi_JetsonNano/c/lib/Config/dev_hardware_SPI.c
    waveshare-lib/RaspberryPi_JetsonNano/c/lib/e-Paper/EPD_7in5_V2.c
)

target_compile_definitions(epd
PUBLIC
    -DRPI=1
    -DUSE_WIRINGPI_LIB=1
)

target_include_directories(epd
PUBLIC
    waveshare-lib/RaspberryPi_JetsonNano/c/lib/e-Paper/
    waveshare-lib/RaspberryPi_JetsonNano/c/lib/Config
)


add_executable(epaper-display
    src/monochromer.cpp
    src/converter.cpp
    src/epaper_display.cpp
    src/filesystem_monitor.cpp
    src/ir_sensor.cpp
    src/main.cpp
)
set_property(TARGET epaper-display PROPERTY CXX_STANDARD 20)

target_include_directories(epaper-display
PUBLIC
    /usr/local/include/
)

target_link_libraries(epaper-display
    epd
    png
    fmt
    wiringPi
)

target_compile_definitions(epaper-display
PUBLIC
    -Dcimg_display=0
)


# A test program to check the monochromer
add_executable(monochromer-only EXCLUDE_FROM_ALL
    src/converter.cpp
    src/filesystem_monitor.cpp
    src/main-monochromer.cpp
    src/monochromer.cpp
)

set_property(TARGET monochromer-only PROPERTY CXX_STANDARD 20)

target_compile_definitions(monochromer-only
PUBLIC
    -Dcimg_display=0
)

target_link_libraries(monochromer-only
    png
    fmt
)

target_include_directories(monochromer-only
PUBLIC
    /usr/local/include/
)

target_link_directories(monochromer-only
PUBLIC
    /usr/local/lib
)

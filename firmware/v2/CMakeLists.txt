cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(nixie_clock)

pico_sdk_init()

add_executable(clock)
pico_generate_pio_header(clock ${CMAKE_CURRENT_LIST_DIR}/spi.pio)
target_sources(clock PRIVATE main.c ds1302.c)
target_link_libraries(clock PRIVATE pico_stdlib hardware_pio)
pico_enable_stdio_usb(clock 1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(clock)

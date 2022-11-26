cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(nixie_clock)

pico_sdk_init()

add_executable(hello)
pico_generate_pio_header(hello ${CMAKE_CURRENT_LIST_DIR}/blink.pio)
target_sources(hello PRIVATE hello.c)
target_link_libraries(hello PRIVATE pico_stdlib hardware_pio)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello)

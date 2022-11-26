cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(nixie_clock)

pico_sdk_init()

add_executable(hello
    hello.c
)

target_link_libraries(hello pico_stdlib)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello)

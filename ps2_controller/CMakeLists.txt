cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(ps2_controller C CXX ASM)

pico_sdk_init()

add_executable(ps2_controller
    src/main.c
    src/usb/usb_descriptors.c
    src/usb/usb_hid.c
    src/ps2/ps2_protocol.c
)

target_include_directories(ps2_controller PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
    ${CMAKE_CURRENT_LIST_DIR}/src/ps2
)

target_link_libraries(ps2_controller
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
)

pico_add_extra_outputs(ps2_controller)
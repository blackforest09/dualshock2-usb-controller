# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(ps2_controller C CXX ASM)

pico_sdk_init()

add_executable(ps2_controller
    src/main.c
    src/usb/usb_descriptors.c
    src/usb/usb_hid.c
    src/dualshock2/ds2_controller.c
)

target_include_directories(ps2_controller PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
    ${CMAKE_CURRENT_LIST_DIR}/src/dualshock2
)

target_link_libraries(ps2_controller
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
    hardware_spi
    hardware_pio
)

pico_enable_stdio_uart(ps2_controller 1)
pico_enable_stdio_usb(ps2_controller 0)

pico_add_extra_outputs(ps2_controller)
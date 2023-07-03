cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico_w)
include(pico_sdk_import.cmake)
project(HoffSolo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

# BTStack/examples compiler options?
add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

add_subdirectory(lib/screen/config)
add_subdirectory(lib/screen/lcd)
add_subdirectory(lib/screen/font)

include_directories(./lib/screen/config)
include_directories(./lib/screen/lcd)
include_directories(./lib/screen/font)
include_directories(./lib/tusb)
include_directories(./lib/hoff)
include_directories(./lib/ble)

# MIDI BLE
add_executable(solo)
target_sources(solo PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/tusb/usb_descriptors.c
        )
target_link_libraries(solo PRIVATE
        pico_stdlib
        pico_btstack_cyw43
        pico_cyw43_arch_threadsafe_background
        pico_btstack_ble
        tinyusb_device
        tinyusb_board
        lcd
        font
        config
        pico_stdlib
        hardware_spi
        hardware_flash
        pico_multicore
        )
target_include_directories(solo PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${PICO_SDK_PATH}/lib/btstack/src
        )
target_compile_definitions(solo PRIVATE
        PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=3000
        CYW43_LWIP=0
        )
pico_add_extra_outputs(solo)

# Generate header from .gatt file, and include the output somehow from the generated folder
pico_btstack_make_gatt_header(solo PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/ble/solo_midi_ble.gatt)

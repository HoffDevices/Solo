cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(HoffSolo)
pico_sdk_init()

add_subdirectory(lib/screen/config)
add_subdirectory(lib/screen/lcd)
add_subdirectory(lib/screen/font)

include_directories(./lib/screen/config)
include_directories(./lib/screen/lcd)
include_directories(./lib/screen/font)
include_directories(./lib/tusb)
include_directories(./lib/hoff)

add_executable(solo 
  main.c
  ./lib/tusb/usb_descriptors.c
)

target_include_directories(solo PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(solo PRIVATE tinyusb_device tinyusb_board lcd font config pico_stdlib hardware_spi hardware_flash pico_multicore)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(solo)

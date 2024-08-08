# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/hunter_h/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(tft_pwm C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(tft_pwm 
  main.cpp 
  driver/st7789.cpp
  driver/read_ad_data.cpp
)

# Init pio
pico_generate_pio_header(tft_pwm ${CMAKE_CURRENT_LIST_DIR}/driver/st7789.pio)
pico_generate_pio_header(tft_pwm ${CMAKE_CURRENT_LIST_DIR}/driver/read_ad_data.pio)

pico_set_program_name(tft_pwm "tft_pwm")
pico_set_program_version(tft_pwm "0.1")

pico_enable_stdio_uart(tft_pwm 1)
pico_enable_stdio_usb(tft_pwm 1)

# Add the standard library to the build
target_link_libraries(tft_pwm
        pico_stdlib
        pico_rand
        )

# Add the standard include files to the build
target_include_directories(tft_pwm PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
  ${CMAKE_CURRENT_LIST_DIR}/driver/..
)

# Add any user requested libraries
target_link_libraries(tft_pwm 
        hardware_spi
        hardware_dma
        hardware_pwm
        hardware_pio
        hardware_gpio
        )

pico_add_extra_outputs(tft_pwm)


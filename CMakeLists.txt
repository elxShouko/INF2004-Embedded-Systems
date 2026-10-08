cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(INF2004_Embedded_Systems C CXX ASM)

pico_sdk_init()

add_executable(INF2004_Embedded_Systems
    src/main.c
    src/obstacle/ultrasonic.c
    src/obstacle/servo.c
    src/obstacle/obstacle.c
)

target_include_directories(
    INF2004_Embedded_Systems
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(
    INF2004_Embedded_Systems
    pico_stdlib
    hardware_pwm
    hardware_clocks
    m
)

pico_enable_stdio_usb(
    INF2004_Embedded_Systems
    1
)

pico_enable_stdio_uart(
    INF2004_Embedded_Systems
    0
)

pico_add_extra_outputs(
    INF2004_Embedded_Systems
)
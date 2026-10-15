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

# Reusable Member 3 algorithms; no motor, network, or RTOS ownership.
add_library(member3_navigation STATIC
    src/line/ir_input.c
    src/line/line_tracking.c
    src/line/junction.c
    src/line/barcode.c
    src/line/navigation_map.c
)
target_include_directories(member3_navigation PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_options(member3_navigation PRIVATE -Wall -Wextra -Werror)

# Member 3 bench firmware is separate from the existing Member 5 executable.
option(BUILD_MEMBER3_IR_DIAGNOSTIC "Build Member 3 IR bench diagnostic" OFF)
if(BUILD_MEMBER3_IR_DIAGNOSTIC)
    add_executable(member3_ir_diagnostic
        testing/subsystem3/diagnostics/member3_ir_diagnostic.c
        src/line/ir_gpio.c
    )
    target_include_directories(member3_ir_diagnostic PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include)
    target_link_libraries(member3_ir_diagnostic pico_stdlib member3_navigation)
    target_compile_options(member3_ir_diagnostic PRIVATE -Wall -Wextra -Werror)
    pico_enable_stdio_usb(member3_ir_diagnostic 1)
    pico_enable_stdio_uart(member3_ir_diagnostic 0)
    pico_add_extra_outputs(member3_ir_diagnostic)
endif()

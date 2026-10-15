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
# ---------------------------------------------------------------------------
# Buddy 4 - IMU-based motion and terrain monitoring
# LSM303DLHC on Robo Pico Grove 4: I2C0, SDA = GP16, SCL = GP17
# ---------------------------------------------------------------------------
# INTERFACE libraries (Pico SDK idiom): sources are compiled into whichever
# executable links them, so no duplicate hardware_* objects are produced.
#
#   buddy4_imu       driver + algorithms + state machine (no RTOS)
#   buddy4_imu_rtos  + the uT-Kernel IMU / Terrain Task wrapper; only defined
#                    when UTKERNEL_INCLUDE_DIRS points at the course
#                    uT-Kernel headers (tk/tkernel.h). The team firmware then
#                    uses:  target_link_libraries(<firmware> buddy4_imu_rtos)
set(UTKERNEL_INCLUDE_DIRS "" CACHE STRING
    "uT-Kernel 3.0 include directories (dirs containing tk/tkernel.h, config.h)")
set(UTKERNEL_DEFINITIONS "" CACHE STRING
    "uT-Kernel target definitions, e.g. _PICO_RP2040_")
option(BUDDY4_RTOS_COMPILE_CHECK
       "Compile imu_rtos.c as an object library (API check without linking)"
       OFF)

add_library(buddy4_imu INTERFACE)
target_sources(buddy4_imu INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_params.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_math.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_hal_pico.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_lsm303dlhc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_calibration.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_processing.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_hump.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_events.c
    ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_terrain.c
)
target_include_directories(buddy4_imu INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(buddy4_imu INTERFACE
    pico_stdlib
    hardware_i2c
    hardware_gpio
    hardware_timer
    m
)

if (UTKERNEL_INCLUDE_DIRS)
    add_library(buddy4_imu_rtos INTERFACE)
    target_sources(buddy4_imu_rtos INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_rtos.c)
    target_include_directories(buddy4_imu_rtos INTERFACE
        ${UTKERNEL_INCLUDE_DIRS})
    # The TRON uT-Kernel headers typedef size_t themselves unless told not
    # to; required when they are mixed with newlib / Pico SDK headers.
    target_compile_definitions(buddy4_imu_rtos INTERFACE
        PROHIBIT_DEF_SIZE_T ${UTKERNEL_DEFINITIONS})
    target_link_libraries(buddy4_imu_rtos INTERFACE buddy4_imu)

    if (BUDDY4_RTOS_COMPILE_CHECK)
        add_library(buddy4_imu_rtos_check OBJECT
            ${CMAKE_CURRENT_LIST_DIR}/src/imu/imu_rtos.c)
        target_include_directories(buddy4_imu_rtos_check PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/include ${UTKERNEL_INCLUDE_DIRS})
        target_compile_definitions(buddy4_imu_rtos_check PRIVATE
            PROHIBIT_DEF_SIZE_T ${UTKERNEL_DEFINITIONS})
        target_compile_options(buddy4_imu_rtos_check PRIVATE -Wall -Wextra)
    endif()
endif()

# Stand-alone bring-up / calibration logger (own main(), no uT-Kernel).
# Does not touch src/main.c or any other buddy's code.
add_executable(buddy4_imu_bringup
    tools/buddy4_bringup/bringup_main.c
)
target_link_libraries(buddy4_imu_bringup buddy4_imu pico_stdlib)
target_compile_options(buddy4_imu_bringup PRIVATE -Wall -Wextra)
pico_enable_stdio_usb(buddy4_imu_bringup 1)
pico_enable_stdio_uart(buddy4_imu_bringup 0)
pico_add_extra_outputs(buddy4_imu_bringup)

# Host-side tests and scope: see testing/subsystem4/TEST_LOG.md
# (they do not use the Pico SDK and are not part of this build).

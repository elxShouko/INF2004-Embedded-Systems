#!/bin/sh
# Build and run the PC unit tests. Needs only gcc (WSL, Linux or macOS).
set -e
cd "$(dirname "$0")"
FLAGS="-std=c99 -Wall -Wextra -Wshadow -Wundef -Wswitch-default -Wfloat-equal"
gcc $FLAGS -o /tmp/s1_codec_host test_codec_host.c \
    ../../../app_program/subsystem1/s1_codec.c
gcc $FLAGS -o /tmp/s1_backoff_host test_backoff_host.c
gcc $FLAGS -Imocks -I../../../lib/liblwip/sysdepend/pico_rp2040 \
    -o /tmp/s1_mqtt_host test_mqtt_host.c \
    ../../../lib/liblwip/sysdepend/pico_rp2040/mqtt_utk.c
/tmp/s1_codec_host
/tmp/s1_backoff_host
/tmp/s1_mqtt_host

# On-target test logic (s1_net.c + subsystem1_test.c) against a PC emulation of
# the kernel calls. The real kernel headers are used, so the types are real.
ROOT=../../..
gcc -std=gnu11 -Wall -Wextra -DAPP_TEST=1 -D_PICO_RP2040_ -DS1_SIMULATE_TELEMETRY=0 -DTM_MQTT=0 \
    -I$ROOT/include -I$ROOT/config -I$ROOT/app_program \
    -o /tmp/s1_target_emulated kernel_emulation.c \
    $ROOT/app_program/subsystem1/s1_net.c $ROOT/app_program/subsystem1/s1_codec.c \
    $ROOT/app_program/subsystem1/s1_sim.c $ROOT/app_program/subsystem1/subsystem1_test.c \
    target_main.c
/tmp/s1_target_emulated

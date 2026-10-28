#!/usr/bin/env bash
set -eu
export PATH="/c/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin:/ucrt64/bin:/usr/bin:$PATH"
export PICO_SDK_PATH=../build/deps/pico-sdk
make "$@" "INCPATH=-I../include -I../config -I../kernel/knlinc -I../build/profiles" -j8

#!/bin/bash
# Before/after evidence for the Minimum-Bit Data Representation table:
# message sizes (real encoders), structure and queue memory (ARM Cortex-M0+
# compiler), and code size of the JSON build versus the binary build.
# Needs gcc, and arm-none-eabi-gcc for the memory part (skipped if absent).
# Run from anywhere:  bash testing/subsystem1/tools/measure_representation.sh
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$HERE/../../..
APP=$ROOT/app_program/subsystem1
OUT=$(mktemp -d)
echo "=== 1. message sizes (host encoders, same code as the firmware)"
gcc -std=c99 -Wall -Wextra -o "$OUT/sizes" "$HERE/../host/measure_sizes_host.c" "$APP/s1_codec.c" && "$OUT/sizes"
echo
ARM=arm-none-eabi-gcc
if ! command -v $ARM >/dev/null 2>&1; then
    echo "NOTE: $ARM not found: memory measurements skipped"
    exit 0
fi
FLAGS="-mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -std=gnu11 -ffunction-sections -fdata-sections -DCNF_SMP=1 -DAPP_TEST=0 -D_PICO_RP2040_ -DTM_MQTT=1 -I$ROOT/lib/liblwip/sysdepend/pico_rp2040 -I$ROOT/include -I$ROOT/config -I$ROOT/kernel/knlinc -I$ROOT/app_program"
cat > "$OUT/sz.c" <<'EOT'
#include "subsystem1/s1_net.h"
#include "subsystem1/s1_codec.h"
char sz_s1_codec_frame_t[sizeof(s1_codec_frame_t)];
char sz_s1_codec_command_t[sizeof(s1_codec_command_t)];
char sz_s1_net_map_request_t[sizeof(s1_net_map_request_t)];
char sz_s1_net_stats_t[sizeof(s1_net_stats_t)];
char sz_binary_telemetry_message[S1_CODEC_TELEMETRY_BIN_BYTES];
EOT
echo "=== 2. structure sizes on the Cortex-M0+ (bytes)"
$ARM $FLAGS -c -o "$OUT/sz.o" "$OUT/sz.c" && arm-none-eabi-nm -S --size-sort -t d "$OUT/sz.o" | awk '{printf "  %-34s %5d\n", $4, $2}'
FRAME=$(arm-none-eabi-nm -S -t d "$OUT/sz.o" | awk '/sz_s1_codec_frame_t/{print $2+0}')
echo
echo "=== 3. telemetry queue memory (16 messages, 8 bytes kernel overhead each)"
echo "  before (queue of s1_codec_frame_t): 16 * ($FRAME + 8) = $((16 * (FRAME + 8))) bytes"
echo "  after  (queue of 29 packed bytes) : 16 * (29 + 8)     = $((16 * (29 + 8))) bytes"
echo "  saving                            : $((16 * (FRAME + 8) - 16 * 37)) bytes"
echo
echo "=== 4. code and static data of the Buddy 1 objects (bytes): JSON build vs binary build"
for MODE in 1 0; do
    for F in s1_codec s1_net; do
        $ARM $FLAGS -DS1_WIRE_JSON=$MODE -c -o "$OUT/${F}_$MODE.o" "$APP/$F.c" 2>/dev/null || echo "compile of $F failed"
    done
    $ARM $FLAGS -DS1_WIRE_JSON=$MODE -c -o "$OUT/s1_map_$MODE.o" "$APP/s1_map.c" 2>/dev/null
    echo "  S1_WIRE_JSON=$MODE ($([ $MODE = 1 ] && echo JSON, the old default || echo binary, the new default))"
    arm-none-eabi-size "$OUT"/s1_codec_$MODE.o "$OUT"/s1_net_$MODE.o "$OUT"/s1_map_$MODE.o 2>/dev/null | sed 's/^/    /'
done
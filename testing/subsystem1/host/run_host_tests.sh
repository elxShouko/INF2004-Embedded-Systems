#!/bin/bash
# Build and run every PC test for subsystem 1. Needs bash and gcc (WSL, Git
# Bash/MinGW, Linux or macOS). The exit status is the verdict: 0 only when
# every build and every test passed. The whole run (commands, revision
# metadata, raw output) is saved to ../logs/host_run_<revision>.txt.
#
#   Optional:  CC=gcc   compiler to use   |   S1_NO_LOG=1   do not save a log
SELF=$(cd "$(dirname "$0")" && pwd)/$(basename "$0")
cd "$(dirname "$SELF")" || exit 2
HERE=$(pwd)
ROOT=$HERE/../../..
LOGDIR=$HERE/../logs

# --- Re-run this script once with all output captured, keeping the status.
if [ -z "$S1_LOGGING" ] && [ -z "$S1_NO_LOG" ]; then
    mkdir -p "$LOGDIR"
    REV=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)
    LOG=$LOGDIR/host_run_$REV.txt
    S1_LOGGING=1 bash "$SELF" "$@" > "$LOG" 2>&1
    STATUS=$?
    cat "$LOG"
    echo "log saved: $LOG (exit status $STATUS)"
    exit $STATUS
fi

CC=${CC:-gcc}
OUT=$(mktemp -d 2>/dev/null || echo "$HERE/.build")
mkdir -p "$OUT"
EXE=""
case "$(uname -s)" in MINGW*|MSYS*|CYGWIN*) EXE=".exe" ;; esac
FAILED=0

echo "=== subsystem 1 host test run ==="
echo "date:     $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "commit:   $(git -C "$ROOT" rev-parse HEAD 2>/dev/null || echo unknown)"
echo "branch:   $(git -C "$ROOT" rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)"
echo "dirty:    $(git -C "$ROOT" status --porcelain 2>/dev/null | grep -v '^??' | wc -l | tr -d ' ') modified tracked file(s)"
echo "host:     $(uname -srm)"
echo "compiler: $($CC --version | head -n 1)"
echo

# Print the command, run it, remember a failure but keep going.
run () {
    echo "+ $*"
    "$@"
    RC=$?
    if [ $RC -ne 0 ]; then
        echo "!! command failed with status $RC"
        FAILED=$((FAILED + 1))
    fi
    return $RC
}

FLAGS="-std=c99 -Wall -Wextra -Wshadow -Wundef -Wswitch-default -Wfloat-equal"
LWIP_DIR=$ROOT/lib/liblwip/sysdepend/pico_rp2040
WIFI_DIR=$ROOT/lib/libwifi/sysdepend/pico_rp2040
APP=$ROOT/app_program/subsystem1

echo "--- 1. pure logic: codec, backoff, MQTT client"
run $CC $FLAGS -o "$OUT/codec$EXE" test_codec_host.c $APP/s1_codec.c && run "$OUT/codec$EXE"
run $CC $FLAGS -o "$OUT/backoff$EXE" test_backoff_host.c && run "$OUT/backoff$EXE"
run $CC $FLAGS -Imocks -I$LWIP_DIR -o "$OUT/mqtt$EXE" test_mqtt_host.c $LWIP_DIR/mqtt_utk.c && run "$OUT/mqtt$EXE"

echo
echo "--- 2. Wi-Fi service task (cyw43_utk.c) against a scripted driver"
run $CC -std=gnu11 -Wall -Wextra -DTM_NET_LWIP=1 -DTM_MQTT=1 -DCNF_SMP=1 \
    -Imocks_wifi -I$WIFI_DIR -I$LWIP_DIR \
    -o "$OUT/wifi$EXE" test_wifi_host.c $WIFI_DIR/cyw43_utk.c && run "$OUT/wifi$EXE"

echo
echo "--- 3. application tests (s1_net.c, s1_sim.c, subsystem1_test.c) on an emulated kernel"
THREADS=""
echo 'int main(void){return 0;}' > "$OUT/probe.c"
if [ -z "$S1_FORCE_NO_THREADS" ] && $CC -pthread -o "$OUT/probe$EXE" "$OUT/probe.c" 2>/dev/null; then
    THREADS="-DHOST_THREADS=1 -pthread"
    SERIAL=""
    echo "pthreads available: the concurrent-status test uses real threads"
else
    SERIAL="-DS1_TEST_SERIAL_HOST=1 -DS1_TEST_REQUIRE_OVERLAP=0"
    echo "NOTE: pthreads not available: the concurrent-status test is LOGIC ONLY here."
    echo "      Run it on the Pico (make TEST=1) for the real two-core evidence."
fi
# PROHIBIT_DEF_SIZE_T: the kernel headers must not define size_t themselves,
# because a hosted libc (MinGW on Windows) already does, with another type.
KFLAGS="-std=gnu11 -Wall -Wextra -Wno-int-to-pointer-cast -DPROHIBIT_DEF_SIZE_T -include stddef.h -DAPP_TEST=1 -D_PICO_RP2040_ -DS1_SIMULATE_TELEMETRY=0 -DTM_MQTT=0 -DS1_TEST_HOST_FAULTS=1"
run $CC -std=gnu11 -Wall -Wextra $THREADS -c -o "$OUT/host_sync.o" host_sync.c
run $CC $KFLAGS -I$ROOT/include -I$ROOT/config -I$ROOT/app_program \
    -o "$OUT/target$EXE" kernel_emulation.c "$OUT/host_sync.o" target_main.c \
    $APP/s1_net.c $APP/s1_codec.c $APP/s1_sim.c $APP/subsystem1_test.c $THREADS $SERIAL \
    && run "$OUT/target$EXE"

echo
echo "--- 4. structure: application code never touches the radio or lwIP"
echo "+ grep for cyw43/lwip use in app_program/subsystem1 (allowed: mqtt_utk_set_app hook registration, net_backoff.h)"
if grep -nE '#include *[<"].*(cyw43|lwip)|[^_a-z]cyw43_[a-z0-9_]+ *\(|[^_a-z](tcp|udp|mqtt)_[a-z_]+ *\(' $APP/*.c $APP/*.h | grep -v 'mqtt_utk_set_app\|net_backoff\.h'; then
    echo "!! application code calls the network stack directly"
    FAILED=$((FAILED + 1))
else
    echo "ok: no direct radio/lwIP/MQTT calls in subsystem1 sources"
fi

echo
echo "--- 5. optional: the same harness built for Windows (MinGW cross compiler, run under wine if present)"
WCC=x86_64-w64-mingw32-gcc
if command -v $WCC >/dev/null 2>&1 && [ "$CC" != "$WCC" ]; then
    WKFLAGS="-std=gnu11 -Wall -Wextra -Wno-int-to-pointer-cast -DPROHIBIT_DEF_SIZE_T -include stddef.h -DAPP_TEST=1 -D_PICO_RP2040_ -DS1_SIMULATE_TELEMETRY=0 -DTM_MQTT=0 -DS1_TEST_HOST_FAULTS=1 -DS1_TEST_SERIAL_HOST=1 -DS1_TEST_REQUIRE_OVERLAP=0"
    run $WCC -std=gnu11 -Wall -Wextra -c -o "$OUT/host_sync_win.o" host_sync.c
    run $WCC $WKFLAGS -I$ROOT/include -I$ROOT/config -I$ROOT/app_program \
        -o "$OUT/target_win.exe" kernel_emulation.c "$OUT/host_sync_win.o" target_main.c \
        $APP/s1_net.c $APP/s1_codec.c $APP/s1_sim.c $APP/subsystem1_test.c
    if command -v wine >/dev/null 2>&1; then
        run env WINEDEBUG=-all wine "$OUT/target_win.exe"
    else
        echo "NOTE: wine not found: the Windows build compiled but was not run."
    fi
else
    echo "skipped: no MinGW cross compiler on this host"
fi

echo
if [ $FAILED -eq 0 ]; then
    echo "RESULT: PASS (all builds succeeded and all tests passed)"
    exit 0
fi
echo "RESULT: FAIL ($FAILED step(s) failed)"
exit 1

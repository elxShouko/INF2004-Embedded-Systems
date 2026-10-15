#!/usr/bin/env bash
# Buddy 4 host regressions against the preserved pre-template implementation.
# Usage: bash testing/subsystem4/run_host_tests.sh [normal|sanitize|replay]
# No hardware, network, checkout, package installation, or integrated Make.
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
repo_dir=$(git -C "$script_dir" rev-parse --show-toplevel)
source_revision=82f2745754ef0c44df0579e3ee4c4f4709e75f47
mode=${1:-normal}
compiler=${CC:-cc}

if [[ $# -gt 1 || ! "$mode" =~ ^(normal|sanitize|replay)$ ]]; then
    printf 'Usage: bash %s [normal|sanitize|replay]\n' "$0" >&2
    exit 2
fi
command -v "$compiler" >/dev/null
git -C "$repo_dir" cat-file -e "${source_revision}^{commit}"

# Unique evidence folders preserve all earlier runs, including failed ones.
run_dir=$(mktemp -d "$script_dir/logs/${mode}.XXXXXX")
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/buddy4-host.XXXXXX")
printf 'Evidence: %s\nTemporary build: %s\n' "$run_dir" "$build_dir"

run_tests()
{
    printf 'Source revision: %s (archived IMU implementation)\n' "$source_revision"
    printf 'Harness HEAD: '
    git -C "$repo_dir" rev-parse HEAD
    printf 'Harness working tree:\n'
    git -C "$repo_dir" status --short
    printf 'Mode: %s\n' "$mode"
    printf 'Recorded at UTC: '
    date -u '+%Y-%m-%dT%H:%M:%SZ'
    "$compiler" --version

    git -C "$repo_dir" archive "$source_revision" src/imu include/imu |
        tar -x -C "$build_dir"

    local sources=()
    local tests=()
    local unit
    for unit in imu_params imu_math imu_lsm303dlhc imu_calibration \
                imu_processing imu_hump imu_events imu_terrain; do
        sources+=("$build_dir/src/imu/$unit.c")
    done
    tests=("$script_dir/host/sim_lsm303.c" "$script_dir/host/scenario.c"
           "$script_dir/host/test_util.c")

    local flags=(-std=c11 -O2 -g -Wall -Wextra -Wpedantic -Wshadow
                 -Wdouble-promotion -Wfloat-equal -Wmissing-prototypes -Werror
                 -DIMU_STATE_TRACE_LEN=64U
                 "-I$build_dir/include" "-I$script_dir/host"
                 "-I$script_dir/host/fake_tk")
    if [[ "$mode" == sanitize ]]; then
        flags+=(-O1 -fsanitize=address,undefined -fno-sanitize-recover=all
                -fno-omit-frame-pointer)
    fi

    if [[ "$mode" == replay ]]; then
        tests+=("$script_dir/host/replay_main.c")
    else
        sources+=("$build_dir/src/imu/imu_rtos.c")
        tests+=("$script_dir/host/fake_tk/fake_tk.c")
        for unit in test_unit_driver test_unit_math_cal test_unit_processing \
                    test_unit_hump test_unit_events test_blackbox \
                    test_robustness test_rtos test_main; do
            tests+=("$script_dir/host/$unit.c")
        done
    fi

    printf 'SHA-256 of harness files (includes uncommitted changes):\n'
    find "$script_dir/host" -type f -print0 |
        LC_ALL=C sort -z | xargs -0 shasum -a 256
    shasum -a 256 "$script_dir/run_host_tests.sh"

    local command=("$compiler" "${flags[@]}" "${sources[@]}" "${tests[@]}"
                   -lm -o "$build_dir/buddy4_tests")
    printf 'Compile command: '
    printf '%q ' "${command[@]}"
    printf '\n'
    "${command[@]}"

    if [[ "$mode" == replay ]]; then
        "$build_dir/buddy4_tests" --gen > "$run_dir/synthetic_raw.txt"
        "$build_dir/buddy4_tests" "$run_dir/synthetic_raw.txt"
    else
        "$build_dir/buddy4_tests"
    fi
}

# pipefail preserves a compiler or test failure even when tee succeeds.
run_tests 2>&1 | tee "$run_dir/console.txt"

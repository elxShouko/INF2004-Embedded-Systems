#!/usr/bin/env bash
# One entry point for the 75 current-firmware core/driver/runtime cases.
# No downloads, hardware access, checkout, or integrated Make invocation.
set -euo pipefail

host_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
test_dir=$(cd -- "$host_dir/.." && pwd)
repo_dir=$(cd -- "$test_dir/../.." && pwd)
mode=${1:-normal}
compiler=${CC:-cc}

if [[ $# -gt 1 || ! "$mode" =~ ^(normal|sanitize)$ ]]; then
    printf 'Usage: bash %s [normal|sanitize]\n' "$0" >&2
    exit 2
fi
command -v "$compiler" >/dev/null
mkdir -p "$test_dir/logs"
run_date=$(TZ=Asia/Singapore date '+%Y-%m-%d')
log_dir=$(mktemp -d "$test_dir/logs/${run_date}_host_${mode}.XXXXXX")
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/subsystem4-host.XXXXXX")

run_current()
{
    local flags=(-std=c99 -Wall -Wextra -Werror -Wconversion -Wshadow
                 -pedantic -O2)
    local case_id
    if [[ "$mode" == sanitize ]]; then
        flags+=(-O1 -g -fsanitize=address,undefined
                -fno-sanitize-recover=all -fno-omit-frame-pointer)
    fi
    printf '\nCURRENT firmware: 75 cases (S4-U116-S4-U166, U176-U199)\n'
    set -x
    "$compiler" "${flags[@]}" \
        "$repo_dir/app_program/subsystem4/imu_core.c" \
        "$repo_dir/app_program/subsystem4/imu_guard.c" \
        "$repo_dir/app_program/subsystem4/drv_lsm.c" \
        "$host_dir/buddy4_cases.c" "$host_dir/host_main.c" \
        -lm -o "$build_dir/current_tests"
    "$build_dir/current_tests"
    "$compiler" "${flags[@]}" -DS4_IMU_ENABLE=1 -I"$host_dir/fake" \
        "$repo_dir/app_program/subsystem4/imu_core.c" \
        "$repo_dir/app_program/subsystem4/imu_runtime.c" \
        "$repo_dir/app_program/subsystem4/imu_guard.c" \
        "$repo_dir/app_program/subsystem4/imu_link.c" \
        "$host_dir/runtime_fake.c" "$host_dir/runtime_main.c" \
        -lm -o "$build_dir/runtime_tests"
    for case_id in {149..161}; do
        "$build_dir/runtime_tests" "$case_id"
    done
    for case_id in {190..199}; do
        "$build_dir/runtime_tests" "$case_id"
    done
    set +x
}

run_tests()
{
    printf 'Subsystem 4 current host suite: mode=%s\n' "$mode"
    printf 'Temporary artifacts: %s\n' "$build_dir"
    date -u '+UTC run time: %Y-%m-%dT%H:%M:%SZ'
    printf 'Harness/current firmware HEAD: '
    git -C "$repo_dir" rev-parse HEAD
    git -C "$repo_dir" status --short
    "$compiler" --version
    find "$repo_dir/app_program/subsystem4" "$host_dir" \
        -type f -exec shasum -a 256 {} +
    shasum -a 256 "$test_dir/TEST_LOG.md"
    run_current
    printf '\nRequested suite completed successfully.\n'
    printf 'Saved evidence: %s/console.txt\n' "$log_dir"
}

# pipefail keeps compiler/test failures visible even when tee succeeds.
run_tests 2>&1 | tee "$log_dir/console.txt"

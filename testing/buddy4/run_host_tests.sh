#!/usr/bin/env bash
# Compile current production sources. No SDK download or hardware access.
set -euo pipefail
test_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "$test_dir/../.." && pwd)"
mode="${1:-normal}"
compiler="${CC:-cc}"
build_dir="$(mktemp -d "${TMPDIR:-/tmp}/buddy4-current.XXXXXX")"
mkdir -p "$test_dir/logs"
log_dir="$(mktemp -d "$test_dir/logs/${mode}.XXXXXX")"
flags=(-std=c99 -Wall -Wextra -Werror -Wconversion -Wshadow -pedantic)
case "$mode" in
    normal) flags+=(-O2) ;;
    sanitize) flags+=(-O1 -g -fsanitize=address,undefined
                      -fno-omit-frame-pointer) ;;
    *) printf 'Usage: bash %s [normal|sanitize]\n' "$0"; exit 2 ;;
esac
exec > >(tee "$log_dir/console.txt") 2>&1
printf 'CURRENT Buddy 4 sources; mode=%s; artifacts=%s\n' "$mode" "$build_dir"
printf 'UTC run time: '
date -u '+%Y-%m-%dT%H:%M:%SZ'
git -C "$repo_dir" rev-parse HEAD
git -C "$repo_dir" status --short
"$compiler" --version
find "$repo_dir/app_program/subsystem4" "$test_dir" -name '*.[ch]' \
    -exec shasum -a 256 {} +
set -x
"$compiler" "${flags[@]}" \
    "$repo_dir/app_program/subsystem4/imu_core.c" \
    "$repo_dir/app_program/subsystem4/drv_lsm.c" \
    "$test_dir/buddy4_cases.c" "$test_dir/host_main.c" \
    -lm -o "$build_dir/buddy4_tests"
"$build_dir/buddy4_tests"
"$compiler" "${flags[@]}" -DS4_IMU_ENABLE=1 -I"$test_dir/fake" \
    "$repo_dir/app_program/subsystem4/imu_core.c" \
    "$repo_dir/app_program/subsystem4/imu_runtime.c" \
    "$test_dir/runtime_fake.c" "$test_dir/runtime_main.c" \
    -lm -o "$build_dir/buddy4_runtime_tests"
for case_id in {149..161}; do
    "$build_dir/buddy4_runtime_tests" "$case_id"
done
set +x
printf 'Saved evidence: %s/console.txt\n' "$log_dir"


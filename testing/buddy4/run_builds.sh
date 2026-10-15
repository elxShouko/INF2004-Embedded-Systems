#!/usr/bin/env bash
# Compile only. Never flash, install tools, or fetch SDK dependencies.
set -euo pipefail
test_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "$test_dir/../.." && pwd)"
mkdir -p "$test_dir/logs"
log_dir="$(mktemp -d "$test_dir/logs/build.XXXXXX")"
exec > >(tee "$log_dir/console.txt") 2>&1
date -u '+UTC run time: %Y-%m-%dT%H:%M:%SZ'
git -C "$repo_dir" rev-parse HEAD
git -C "$repo_dir" status --short
arm-none-eabi-gcc --version
printf 'PICO_SDK_PATH=%s\n' "${PICO_SDK_PATH:-unset}"
find "$repo_dir/app_program/subsystem4" "$test_dir" -name '*.[ch]' \
    -exec shasum -a 256 {} +
shasum -a 256 "$repo_dir/build_make/makefile" \
    "$repo_dir/build_make/pico_rp2040.mk"
cd "$repo_dir/build_make"
set -x
make -s -j8
make -s TEST=1 -j8
make -s S4_IMU=1 -j8
make -s TEST=1 S4_IMU=1 -j8
set +x
shasum -a 256 mtk3pico_smp1_usb_cdc_wifi*.uf2
printf 'Saved build evidence: %s/console.txt\n' "$log_dir"


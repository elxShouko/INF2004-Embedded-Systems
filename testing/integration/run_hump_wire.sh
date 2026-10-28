#!/usr/bin/env bash
# Host-only INT-86: Buddy 4 height to Buddy 1 binary telemetry boundary.
set -euo pipefail

test_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
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
log_dir=$(mktemp -d "$test_dir/logs/${run_date}_int86_${mode}.XXXXXX")
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/int86-host.XXXXXX")
flags=(-std=c99 -Wall -Wextra -Werror -Wconversion -Wshadow -pedantic
       -O2 -ffunction-sections -fdata-sections)
if [[ "$mode" == sanitize ]]; then
    flags+=(-O1 -g -fsanitize=address,undefined
            -fno-sanitize-recover=all -fno-omit-frame-pointer)
fi
link_flags=(-Wl,--gc-sections)
if [[ "$(uname -s)" == Darwin ]]; then
    link_flags=(-Wl,-dead_strip)
fi

run_case()
{
    printf 'INT-86 mode=%s\n' "$mode"
    printf 'HEAD: '
    git -C "$repo_dir" rev-parse HEAD
    git -C "$repo_dir" status --short
    "$compiler" --version
    shasum -a 256 "$repo_dir/app_program/vehicle.c" \
        "$test_dir/test_hump_wire.c"
    "$compiler" "${flags[@]}" -I"$repo_dir/app_program" \
        "$repo_dir/app_program/vehicle.c" "$test_dir/test_hump_wire.c" \
        "${link_flags[@]}" -lm -o "$build_dir/int86"
    "$build_dir/int86"
    printf 'Saved evidence: %s/console.txt\n' "$log_dir"
}

run_case 2>&1 | tee "$log_dir/console.txt"

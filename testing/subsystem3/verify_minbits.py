"""Measure S3 ABI/allocation changes without running hardware or Make.

The baseline directory must be an extracted, identified Git revision. This
script compiles probes only into ignored build scratch and never edits sources.
"""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def run(command, cwd):
    result = subprocess.run([str(part) for part in command], cwd=cwd,
                            capture_output=True, text=True, timeout=60)
    entry = {"command": [str(part) for part in command],
             "exit": result.returncode, "stdout": result.stdout,
             "stderr": result.stderr}
    if result.returncode:
        raise RuntimeError(json.dumps(entry, indent=2))
    return entry


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--baseline-root", type=Path, required=True)
    parser.add_argument("--log", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    baseline = args.baseline_root.resolve()
    scratch = root / "build/minbits-measurements"
    scratch.mkdir(parents=True, exist_ok=True)
    arm = Path("C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/"
               "gcc-arm-none-eabi/bin/arm-none-eabi-gcc.exe")
    nm = arm.with_name("arm-none-eabi-nm.exe")
    host = Path("C:/msys64/ucrt64/bin/gcc.exe")
    hostnm = host.with_name("nm.exe")
    types = ["ir_sample_t", "ir_filter_t", "barcode_capture_t",
             "junction_detector_t", "nav_map_t", "nav_event_t", "s3_config_t",
             "s3_input_t", "s3_output_t", "s3_status_t", "s3_context_t"]
    source = '#include "member3_navigation.c"\n' + "\n".join(
        f'unsigned char abi_{name}[sizeof({name})];' for name in types)
    probe = scratch / "sizes.c"
    probe.write_text(source + "\n")
    fixture = root / "testing/subsystem3/member3_rtos_test.c"
    host_header = scratch / "host_types.h"
    host_header.write_text(fixture.read_text().split(
        '#include "member3_navigation.c"')[0])
    report = {
        "baseline_ref": "d4c91f271c9801802e3994967c96e9ec8fcba38c",
        "baseline_root": str(baseline),
        "head": run(["git", "rev-parse", "HEAD"], root)["stdout"].strip(),
        "status": run(["git", "status", "--short"], root)["stdout"],
        "scope": "Compiler ABI and requested allocation only; no target timing",
        "checks": [], "sizes": {}, "sha256": {},
    }
    for version, tree in [("before", baseline), ("after", root)]:
        paths = sorted((tree / "app_program/subsystem3").rglob("*"))
        report["sha256"][version] = {
            path.relative_to(tree).as_posix():
            hashlib.sha256(path.read_bytes()).hexdigest()
            for path in paths if path.suffix in {".c", ".h"}}
        for platform, compiler, symbol_tool in [
                ("arm", arm, nm), ("host", host, hostnm)]:
            output = scratch / f"{version}_{platform}.o"
            flags = ["-std=gnu11", "-O2", "-Wall", "-Wextra", "-Werror",
                     "-I" + str(tree / "app_program/subsystem3")]
            if platform == "arm":
                flags += ["-mcpu=cortex-m0plus", "-mthumb", "-ffreestanding",
                          "-D_PICO_RP2040_", "-DCNF_SMP=1", "-DAPP_TEST=0",
                          "-DTM_CONSOLE_UART=0", "-DBOARD_UART0_FREE=1",
                          "-I" + str(tree / "include"),
                          "-I" + str(tree / "config"),
                          "-I" + str(tree / "kernel/knlinc"), "-fstack-usage"]
            else:
                flags += ["-include", str(host_header)]
            if platform == "host":
                output = output.with_suffix(".s")
                report["checks"].append(run(
                    [compiler, *flags, "-S", probe, "-o", output], root))
                sizes = {name: int(size) for name, size in re.findall(
                    r"^_?abi_(\w+):\s*\n\s*\.space\s+(\d+)",
                    output.read_text(), re.M)}
            else:
                report["checks"].append(run(
                    [compiler, *flags, "-c", probe, "-o", output], root))
                symbols = run([symbol_tool, "-S", "--radix=d", output], root)
                report["checks"].append(symbols)
                sizes = {name: int(size) for size, name in re.findall(
                    r"^\d+\s+(\d+)\s+\w\s+abi_(\w+)\s*$",
                    symbols["stdout"], re.M)}
                usage = output.with_suffix(".su")
                report.setdefault("stack_usage", {})[version] = usage.read_text()
                if version == "after":
                    codec = tree / "app_program/subsystem3/drv_s3_codec.c"
                    codec_obj = scratch / "codec.o"
                    report["checks"].append(run(
                        [compiler, *flags, "-c", codec, "-o", codec_obj], root))
                    report["stack_usage"]["codec"] = (
                        codec_obj.with_suffix(".su")).read_text()
            report["sizes"][f"{version}_{platform}"] = sizes
            if set(sizes) != set(types):
                raise RuntimeError("Incomplete symbol-size report")
    unchanged = {}
    for path in sorted((root / "app_program").rglob("*")):
        if path.suffix not in {".c", ".h"} or "subsystem3" in path.parts:
            continue
        relative = path.relative_to(root)
        same = path.read_bytes().replace(b"\r\n", b"\n") == (
            baseline / relative).read_bytes().replace(b"\r\n", b"\n")
        unchanged[relative.as_posix()] = same
    report["unchanged_other_application_sources"] = unchanged
    if not all(unchanged.values()):
        raise RuntimeError("Unexpected change outside subsystem3")
    before = report["sizes"]["before_arm"]
    after = report["sizes"]["after_arm"]
    report["queue_allocation"] = {
        "before": 4 * (before["s3_input_t"] + 4) +
                  16 * (before["s3_output_t"] + 4),
        "after": 4 * (80 + 4) + 16 * (20 + 4),
        "note": "Actual create requests checked by S3-U29; excludes heap headers",
    }
    report["queue_allocation"]["saved"] = (
        report["queue_allocation"]["before"] -
        report["queue_allocation"]["after"])
    report["message_bytes"] = {
        "config": 79, "mode": 5, "mapping": 18, "odometry": 21,
        "manoeuvre_done": 9, "navigation": 19, "line": 18, "reply": 13}
    for name in types:
        print(f'{name}: ARM {before[name]} -> {after[name]} bytes')
    print("Queue allocations:", report["queue_allocation"])
    args.log.parent.mkdir(parents=True, exist_ok=True)
    args.log.write_text(json.dumps(report, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

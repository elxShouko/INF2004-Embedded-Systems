"""Run existing logical/algorithm suites plus the separate 10 ms host suite.

Uses installed tools only; no Make, downloads, flashing or hardware access.
"""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys


def run(command):
    try:
        result = subprocess.run([str(part) for part in command],
                                capture_output=True, text=True, timeout=60)
    except subprocess.TimeoutExpired:
        return {"command": [str(part) for part in command], "exit": 124,
                "stdout": "", "stderr": "Timed out after 60 seconds\n"}
    return {"command": [str(part) for part in command],
            "exit": result.returncode,
            "stdout": result.stdout, "stderr": result.stderr}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--gcc", default="gcc")
    parser.add_argument("--log", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    source = root / "app_program/subsystem3"
    output = root / "build/member3-production"
    output.mkdir(parents=True, exist_ok=True)
    tick = re.search(r"^#define\s+CNF_TIMER_PERIOD\s+(\d+)",
                     (root / "config/config.h").read_text(), re.M)
    if tick is None or int(tick[1]) != 10:
        raise RuntimeError("Production fixture requires the current 10 ms tick")
    paths = sorted(set(source.rglob("*.c")) | set(source.rglob("*.h")) |
                   set((root / "testing/subsystem3").rglob("*.c")) |
                   {Path(__file__).resolve(), root / "config/config.h"})
    hashes = {str(path.relative_to(root)).replace("\\", "/"):
              hashlib.sha256(path.read_bytes()).hexdigest() for path in paths}
    report = {
        "tested_head": run(["git", "rev-parse", "HEAD"])["stdout"].strip(),
        "status": run(["git", "status", "--short"])["stdout"],
        "production_tick_ms": int(tick[1]), "file_sha256": hashes,
        "scope": "Host simulation only; no target scheduling or physical proof",
        "checks": [],
    }
    report["checks"].append(run([sys.executable,
                                  root / "testing/subsystem3/check_barr_style.py"]))
    suites = {
        "ir": ["ir_input"],
        "navigation": ["ir_input", "barcode", "navigation_map", "line_tracking"],
        "junction": ["ir_input", "line_tracking", "junction"],
        "rtos": ["ir_input", "line_tracking", "barcode", "navigation_map", "junction"],
        "production": ["ir_input", "line_tracking", "barcode", "navigation_map", "junction"],
        "feedback": ["ir_input", "line_tracking", "barcode", "navigation_map", "junction"],
        "minbits": ["ir_input", "line_tracking", "barcode", "navigation_map", "junction"],
        "safety": ["ir_input", "line_tracking", "barcode", "navigation_map", "junction"],
        "requirements": ["ir_input", "line_tracking", "barcode", "navigation_map", "junction"],
    }
    for suite, modules in suites.items():
        if suite in {"rtos", "production", "feedback", "minbits", "safety", "requirements"}:
            modules = modules + ["s3_codec"]
        executable = output / f"{suite}.exe"
        test = root / (f"testing/subsystem3/member3_{suite}_test.c"
                       if suite in {"rtos", "production", "feedback", "minbits", "safety", "requirements", "navigation"} else
                       f"testing/subsystem3/logs/member3_{suite}_test.c")
        command = [args.gcc, "-std=c99", "-Wall", "-Wextra", "-Werror",
                   "-I" + str(source), test]
        if suite in {"minbits", "safety"}:
            command += ["-O2", "-Wconversion", "-Wsign-conversion",
                        "-pedantic", "-fanalyzer"]
        command += [source / f"drv_{module}.c" for module in modules]
        if suite == "requirements":
            command += [root / "app_program/subsystem1/s1_codec.c"]
        command += ["-o", executable]
        compiled = run(command)
        report["checks"].append(compiled)
        if compiled["exit"] == 0:
            report["checks"].append(run([executable]))
    target = args.log or output / "results.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    report["passed"] = all(item["exit"] == 0 for item in report["checks"])
    target.write_text(json.dumps(report, indent=2) + "\n")
    for item in report["checks"]:
        print(item["stdout"], end="")
        print(item["stderr"], end="", file=sys.stderr)
    print("PASS" if report["passed"] else "FAIL", "Evidence:", target)
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())

"""Re-run host, ABI and differential checks without modifying the firmware.

Build outputs go under ignored build/. Source snapshots are read from Git;
this script never switches branches, stages files, or accesses the network.
"""

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys


def run(command):
    result = subprocess.run([str(part) for part in command], capture_output=True, text=True)
    record = {"command": [str(part) for part in command],
              "exit": result.returncode, "stdout": result.stdout,
              "stderr": result.stderr}
    if result.returncode:
        raise RuntimeError(json.dumps(record, indent=2))
    return record


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--gcc", default="gcc")
    parser.add_argument("--baseline", default="a508994331acf0adaa9acff1927cdaeb9f232abe")
    parser.add_argument("--log", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    source = root / "app_program/subsystem3"
    output = root / "build/barr-verification"
    baseline = output / "baseline/app_program/subsystem3"
    baseline.mkdir(parents=True, exist_ok=True)
    files = run(["git", "ls-tree", "-r", "--name-only", args.baseline,
                 "--", "app_program/subsystem3"])["stdout"].splitlines()
    for name in files + ["app_program/app.h"]:
        destination = output / "baseline" / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        result = subprocess.run(["git", "show", f"{args.baseline}:{name}"],
                                capture_output=True, check=True)
        destination.write_bytes(result.stdout)
    fingerprint = hashlib.sha256()
    for path in sorted(source.rglob("*")):
        if path.suffix in {".c", ".h"} or path.name == ".clang-format":
            fingerprint.update(str(path.relative_to(source)).replace("\\", "/").encode())
            fingerprint.update(path.read_bytes())
    report = {"baseline": args.baseline,
              "tested_head": run(["git", "rev-parse", "HEAD"])["stdout"].strip(),
              "working_tree_source_sha256": fingerprint.hexdigest(),
              "status": run(["git", "status", "--short"])["stdout"],
              "checks": []}
    report["checks"].append(run([sys.executable, root / "testing/subsystem3/check_barr_style.py"]))
    suites = {
        "ir": ["ir_input"],
        "navigation": ["ir_input", "barcode", "navigation_map", "line_tracking"],
        "junction": ["ir_input", "line_tracking", "junction"],
        "rtos": ["ir_input", "line_tracking", "barcode", "navigation_map", "junction"],
    }
    flags = [args.gcc, "-std=c99", "-Wall", "-Wextra", "-Werror"]
    for version, folder in [("before", baseline), ("after", source)]:
        for suite, modules in suites.items():
            executable = output / f"{version}-{suite}.exe"
            command = flags + ["-I" + str(folder),
                root / f"testing/subsystem3/logs/member3_{suite}_test.c"]
            command += [folder / f"drv_{module}.c" for module in modules]
            command += ["-o", executable]
            report["checks"].append(run(command))
            report["checks"].append(run([executable]))
    harness = (root / "testing/subsystem3/logs/member3_rtos_test.c").read_text()
    prefix = harness.split('#include "member3_navigation.c"')[0]
    abi_source = output / "abi.c"
    abi = prefix + '\n#include "member3_navigation.h"\nint main(void)\n{\n'
    types = ["ir_sample_t", "ir_calibration_t", "ir_filter_t", "barcode_capture_t",
             "junction_config_t", "junction_sample_t", "junction_detector_t",
             "s3_config_t", "s3_input_t", "s3_output_t", "s3_status_t"]
    for name in types:
        abi += f'printf("{name}=%zu\\n", sizeof({name}));\n'
    for name, member in [("s3_input_t", "value"), ("s3_output_t", "navigation"),
                         ("s3_status_t", "mapping")]:
        abi += f'printf("{name}.{member}=%zu\\n", offsetof({name}, {member}));\n'
    abi += 'printf("enums=%u,%u,%u\\n", (unsigned)NAV_COMMAND_U_TURN, (unsigned)S3_INPUT_MANOEUVRE_DONE, (unsigned)NAV_EVENT_FAULT);\nreturn 0;\n}\n'
    abi_source.write_text(abi)
    trace_source = output / "trace.c"
    trace = '#define main retained_unit_main\n' + harness + '\n#undef main\n'
    trace += r'''
static uint64_t g_trace_hash = UINT64_C(1469598103934665603);
static void trace_word(uint64_t value)
{
    g_trace_hash ^= value;
    g_trace_hash *= UINT64_C(1099511628211);
}
int main(void)
{
    uint32_t random = UINT32_C(0x12345678);
    for (unsigned int segment = 0; segment < 200; ++segment)
    {
        s3_context_t context;
        init_context(&context);
        for (unsigned int sample = 0; sample < 200; ++sample)
        {
            random ^= random << 13;
            random ^= random >> 17;
            random ^= random << 5;
            uint8_t mask = (random % 11U == 0U) ? (uint8_t)(random & 7U) : 3U;
            mask |= (uint8_t)((random & 1U) << 2U);
            tick(&context, mask);
            trace_word(context.status.state);
            trace_word(context.status.mode);
            trace_word(context.status.fault);
            trace_word(context.status.line_valid);
            trace_word(context.status.line);
            trace_word(context.status.black_mask);
            trace_word(context.status.pending_manoeuvre_event_id);
            trace_word(context.status.output_drops);
            trace_word(g_count);
            for (unsigned int event = 0; event < g_count; ++event)
            {
                trace_word(g_outputs[event].event_id);
                trace_word(g_outputs[event].kind);
                trace_word(g_outputs[event].navigation.kind);
                trace_word(g_outputs[event].navigation.command);
                trace_word(g_outputs[event].navigation.timestamp_us);
            }
            g_count = 0;
        }
    }
    printf("40000 samples; trace=%016llx\n", (unsigned long long)g_trace_hash);
    return 0;
}
'''
    trace_source.write_text(trace)
    for kind, input_file in [("abi", abi_source), ("trace", trace_source)]:
        outputs = []
        for version, folder in [("before", baseline), ("after", source)]:
            executable = output / f"{version}-{kind}.exe"
            command = flags + ["-I" + str(folder), input_file]
            if kind == "trace":
                command += [folder / f"drv_{module}.c" for module in suites["rtos"]]
            command += ["-o", executable]
            report["checks"].append(run(command))
            result = run([executable])
            report["checks"].append(result)
            outputs.append(result["stdout"])
        if outputs[0] != outputs[1]:
            raise RuntimeError(f"{kind} changed: {outputs}")
        report[kind + "_comparison"] = {"equal": True, "output": outputs[1]}
    target = args.log or output / "host-results.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(report, indent=2) + "\n")
    print("PASS: baseline and edited host suites, public ABI, and differential trace.")
    print(report["trace_comparison"]["output"].strip())
    print("Working-tree source SHA256:", report["working_tree_source_sha256"])
    print("Evidence:", target)


if __name__ == "__main__":
    main()

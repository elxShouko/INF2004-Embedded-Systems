"""Compare nominal S3 traces against an identified, saved working-tree baseline.

No Make, tool installation, branch changes, hardware or target timing claims.
"""

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time


TRACE = r'''
#define main retained_owner_main
#include "member3_rtos_test.c"
#undef main
static uint64_t g_hash = UINT64_C(1469598103934665603);
static uint32_t g_samples;
static uint32_t g_events;
static void word(uint64_t value)
{
    g_hash ^= value;
    g_hash *= UINT64_C(1099511628211);
}
static void observe(s3_context_t *context)
{
    word(context->status.state);
    word(context->status.mode);
    word(context->status.fault);
    word(context->status.line_valid);
    word(context->status.line);
    word(context->status.black_mask);
    word(context->status.raw.timestamp_us);
    word(context->status.pending_manoeuvre_event_id);
    word(context->status.output_drops);
    for (uint32_t index = 0; index < NAV_ALPHABET_SIZE; ++index)
        word(nav_action_get(context->map.actions, index));
    word(g_count);
    for (uint32_t index = 0; index < g_count; ++index)
    {
        const s3_output_t *event = &g_outputs[index];
        word(event->event_id); word(event->kind);
        word(event->navigation.kind); word(event->navigation.command);
        word(event->navigation.timestamp_us); word(event->navigation.state);
        word(event->navigation.black_mask);
        word((uint8_t)event->navigation.barcode_character);
        word(event->line); word(event->map_revision); word(event->request_id);
        word(event->accepted); word(event->fault);
    }
    g_events += g_count;
    g_count = 0;
}
static void sample(s3_context_t *context, uint8_t mask)
{
    tick(context, mask);
    ++g_samples;
    observe(context);
}
static void barcode(s3_context_t *context)
{
    const char *symbols[] = {"nWnnWnWnn", "WnnnnWnnW", "nWnnWnWnn"};
    for (uint32_t count = 0; count < 12; ++count) sample(context, 3);
    bool black = true;
    for (uint32_t symbol = 0; symbol < 3; ++symbol)
    {
        for (uint32_t element = 0; element < 9; ++element)
        {
            uint32_t width = symbols[symbol][element] == 'W' ? 3U : 1U;
            for (uint32_t count = 0; count < width; ++count)
                sample(context, black ? 7 : 3);
            black = !black;
        }
        if (symbol < 2) { sample(context, 3); black = true; }
    }
    for (uint32_t count = 0; count < 12; ++count) sample(context, 3);
}
int main(void)
{
    mbf_s3_input = 2; mbf_s3_output = 3;
    uint32_t random = UINT32_C(0x12345678);
    for (uint32_t segment = 0; segment < 200; ++segment)
    {
        s3_context_t context;
        init_context(&context);
        for (uint32_t count = 0; count < 200; ++count)
        {
            random ^= random << 13; random ^= random >> 17;
            random ^= random << 5;
            uint8_t mask = random % 11U == 0 ? (uint8_t)(random & 7U) : 3;
            mask |= (uint8_t)((random & 1U) << 2U);
            sample(&context, mask);
        }
    }
    for (uint32_t direction = 0; direction < 4; ++direction)
    {
        s3_context_t context;
        init_context(&context);
        const char *maps[] = {"MAP A=LEFT", "MAP A=RIGHT",
                             "MAP A=FORWARD", "MAP A=UTURN"};
        s3_input_t request = {0}; request.kind = S3_INPUT_MAPPING;
        assert(nav_update_parse(&request.value.mapping, maps[direction]));
        input(&context, &request, 0); observe(&context);
        barcode(&context);
        uint32_t pending = context.status.pending_manoeuvre_event_id;
        assert(pending != 0);
        request.kind = S3_INPUT_MANOEUVRE_DONE;
        request.value.manoeuvre_event_id = pending + 1;
        input(&context, &request, context.status.raw.timestamp_us);
        observe(&context);
        request.value.manoeuvre_event_id = pending;
        input(&context, &request, context.status.raw.timestamp_us);
        observe(&context);
        for (uint32_t count = 0; count < 8; ++count) sample(&context, 3);
        assert(context.status.mode == LINE_MODE_PAUSED);
    }
    printf("S3-U36 samples=%u outputs=%u hash=%016llx\n", g_samples,
           g_events, (unsigned long long)g_hash);
    return 0;
}
'''


def run(command, root):
    started = time.perf_counter()
    result = subprocess.run([str(part) for part in command], cwd=root,
                            capture_output=True, text=True, timeout=60)
    return {"command": [str(part) for part in command],
            "exit": result.returncode, "stdout": result.stdout,
            "stderr": result.stderr,
            "host_elapsed_s": time.perf_counter() - started}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--before", type=Path, required=True)
    parser.add_argument("--gcc", default="C:/msys64/ucrt64/bin/gcc.exe")
    parser.add_argument("--log", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    scratch = root / "build/s3-feedback-comparison"
    scratch.mkdir(parents=True, exist_ok=True)
    trace = scratch / "trace.c"
    trace.write_text(TRACE)
    report = {"baseline": json.loads((args.before / "snapshot.json").read_text()),
              "head": run(["git", "rev-parse", "HEAD"], root)["stdout"].strip(),
              "scope": "Deterministic host equivalence; elapsed host process time "
                       "is informational, not target latency/WCET evidence",
              "checks": [], "profiles": {}, "current_sha256": {}}
    for relative, expected in report["baseline"]["sha256"].items():
        path = args.before / "subsystem3" / Path(relative).relative_to(
            "app_program/subsystem3")
        if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            raise RuntimeError("Baseline source fingerprint mismatch: " + relative)
    for path in sorted((root / "app_program/subsystem3").rglob("*")):
        if path.suffix in {".c", ".h"}:
            report["current_sha256"][path.relative_to(root).as_posix()] = (
                hashlib.sha256(path.read_bytes()).hexdigest())
    for profile, flags in [("default", []), ("optimized", ["-O2", "-fshort-enums"])]:
        traces = []
        for version, source in [("before", args.before / "subsystem3"),
                                ("after", root / "app_program/subsystem3")]:
            executable = scratch / f"{profile}_{version}.exe"
            command = [args.gcc, "-std=c99", "-Wall", "-Wextra", "-Werror",
                       *flags, "-I" + str(source),
                       "-I" + str(root / "testing/subsystem3"), trace]
            command += [source / f"drv_{name}.c" for name in
                        ["ir_input", "line_tracking", "barcode", "navigation_map",
                         "junction", "s3_codec"]]
            built = run([*command, "-o", executable], root)
            report["checks"].append(built)
            if built["exit"]:
                raise RuntimeError(built["stderr"])
            result = run([executable], root)
            report["checks"].append(result)
            if result["exit"]:
                raise RuntimeError(result["stderr"])
            traces.append(result["stdout"])
        report["profiles"][profile] = {"equal": traces[0] == traces[1],
                                        "before": traces[0], "after": traces[1]}
        print(profile, "PASS" if traces[0] == traces[1] else "FAIL",
              traces[1].strip())
    report["passed"] = all(value["equal"] for value in report["profiles"].values())
    args.log.parent.mkdir(parents=True, exist_ok=True)
    args.log.write_text(json.dumps(report, indent=2) + "\n")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

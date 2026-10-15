# Subsystem 4 — test log

Historical records are preserved below. Current firmware tests and new results
are maintained in [`testing/buddy4/TEST_LOG.md`](../buddy4/TEST_LOG.md).

**Subsystem:** Buddy 4 IMU / terrain tests and retained monitor example  ·
**Student ID:** 2501284 (Eddy Teo Hon Kai)

The current firmware code in `app_program/subsystem4/` is the retained monitor.
The new host suite tests archived IMU revision
`82f2745754ef0c44df0579e3ee4c4f4709e75f47`. See `README.md` for the exact scope.

See `testing/README.md` for how to run tests and record results.

## Test cases

Define a test here before running it. Test IDs never change.

| ID | What is tested | Requirement / interface | Method | Steps / how | Expected result |
|---|---|---|---|---|---|
| S4-U01 | message buffer max line = S4_SUMMARY_MAX | IF-03 | auto | `subsystem4_test.c` | `[TEST] S4-U01 PASS` |
| S4-U02 | LED pauses and resumes every 6 reports | IF-05 | manual | 1. Flash normal image. 2. Watch LED and console ~20 s. | console shows "LED task paused"/"resumed"; LED stops/starts |

*Method:* **auto** = `TEST_CHECK` in code, result printed on the console;
**manual** = a person observes the result (LED, scope, meter, phone app…).

### Host cases added before execution

S4-U003–S4-U115 are defined individually in [CASES.md](CASES.md), including
source locations, expected results, methods, and requirement traceability.
Run `bash testing/subsystem4/run_host_tests.sh normal` or `sanitize` from the
repository root. `replay` is a synthetic demonstration and receives no test ID.
All fixture tolerances are regression criteria, not physical acceptance limits.

The six sequential wrapper cases characterize the archived wrapper. They do
not verify its SMP behavior, real kernel scheduling, or required startup order.

## Results

Add a new row for every run. Never edit or delete old rows.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| YYYY-MM-DD | S4-U01 | `abc1234` | | PASS / FAIL | `logs/YYYY-MM-DD_….txt` | |
| 2026-10-15 | S4-U003–S4-U115 | Subject `82f2745`; harness `a405c5a` + uncommitted test-folder changes | Codex | PASS, host simulation | [normal console](logs/normal.fsXov0/console.txt) | 113 tests, 2939 assertions, 0 failures; Apple Clang 21, C11, O2, warnings treated as errors. |
| 2026-10-15 | S4-U003–S4-U115 | Subject `82f2745`; same harness working tree | Codex | PASS, host simulation with ASan + UBSan | [sanitizer console](logs/sanitize.FqCkMD/console.txt) | 113 tests, 2939 assertions, 0 failures; no sanitizer diagnostics. |

Exact commands from repository root:

```sh
bash testing/subsystem4/run_host_tests.sh normal
bash testing/subsystem4/run_host_tests.sh sanitize
bash testing/subsystem4/run_host_tests.sh replay
bash -n testing/subsystem4/run_host_tests.sh
git diff --check
```

The replay command completed with 1100 synthetic rows and three detected humps;
it is a demonstration, not a physical accuracy result. See
[replay console](logs/replay.5koR9p/console.txt). The source revision in full is
`82f2745754ef0c44df0579e3ee4c4f4709e75f47`; harness base HEAD is
`a405c5aa2752e39bdcd3d4e0b415d2126158ea15`. Each console records SHA-256 hashes
of the harness code and runner, so uncommitted test changes are identifiable.
The empty-filter diagnostic was also checked: `buddy4_tests -f no_such_s4_case`
printed `No tests matched the filter.` and returned 2.

No commit was made because commit approval was not given. This records the
working tree explicitly instead of claiming the test harness is committed, as
normally requested by `testing/README.md`. The old example result row above is
a preserved placeholder, not evidence. No integrated Make build, on-target
RTOS test, wiring test, calibration run, or physical test was performed. This
change only affects tests/documents; the six fake-kernel tests do not validate
SMP or the required integrated startup sequence.

# System — test log

Tests of the complete product against its requirements, run on the final hardware set-up. Run the full set before every demo and before hand-in.

See `testing/README.md` for how to run tests and record results.

## Test cases

Define a test here before running it. Test IDs never change.

| ID | What is tested | Requirement / interface | Method | Steps / how | Expected result |
|---|---|---|---|---|---|
| SYS-01 | e.g. device reports a reading over WiFi every 10 s | REQ-? | manual | 1. Power on. 2. Watch the dashboard for 2 minutes. | a reading appears every 10 s ± 1 s |
| SYS-02 | | | manual | 1. … 2. … | |

*Method:* **auto** = `TEST_CHECK` in code, result printed on the console;
**manual** = a person observes the result (LED, scope, meter, phone app…).

## Results

Add a new row for every run. Never edit or delete old rows.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| YYYY-MM-DD | SYS-01 | `abc1234` | | PASS / FAIL | `logs/YYYY-MM-DD_….txt` | |

## Staged production hardware acceptance definitions 2026-10-28

These cases are procedures only. Every case below is NOT RUN: no board was flashed, debug-attached or actuated during software implementation. Existing placeholder SYS-01/02 rows remain historical placeholders.

| ID | Procedure | Expected result |
|---|---|---|
| SYS-03 | Flash exact BENCH profile after separate authorisation; motor power inhibited | No motor arm/drive; USB reports stage, unknown configuration, real IR/IMU state and faults |
| SYS-04 | Rotate each raised wheel manually | Correct independent raw x4 counts; record count/revolution and direction evidence |
| SYS-05 | Configure reviewed bench limits, authorise power, ARM then PULSE | One bounded pulse; both outputs stop; measure actual direction, pulse and E-STOP latency |
| SYS-06 | Stop lease monitor/disconnect network during authorised bench pulse | Local timeout stops; capture physical latency and independent power-removal readiness |
| SYS-07 | Measured LINE profile, flat clear track, bounded START | Production controller/PID follows line; stops at markings/obstacles, lost feedback, timeout or E-STOP |
| SYS-08 | Calibrated BARCODE profile and runtime map | All four manoeuvres at correct junction origin, completion correlation and original-line recovery |
| SYS-09 | Known hump passes at controlled speeds | Detect hump, establish height accuracy/repeatability and correct highest peak with justified traction trust |
| SYS-10 | Full course with approved obstacle clearance source | Safe bypass, original-line reacquisition, all navigation objectives and Wi-Fi/MQTT reporting with saved evidence |

Record exact source revision/uncommitted changes, profile/firmware hash, physical setup, measurements and errors. Add results and video evidence only after execution. See `docs/commissioning.md`.

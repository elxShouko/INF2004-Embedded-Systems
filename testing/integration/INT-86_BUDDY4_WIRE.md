# INT-86 — Buddy 4 to Buddy 1 hump wire boundary

The shared controller converts Buddy 4's floating-point highest hump in mm
to Buddy 1's 12-bit binary telemetry field (valid range 0–4095 mm). This
integration case checks the original float **before** integer conversion.

| ID | Method | Inputs | Expected result |
|---|---|---|---|
| INT-86 | `test_hump_wire.c`, host | 0.0, 4095.0, 4095.9, 4096.0, -1.0, NaN and infinity mm; otherwise fresh/healthy IMU snapshot | 0.0 and 4095.0 mm set `S1_VALID_HUMP`; an out-of-range value is never silently truncated into range. `S1_VALID_IMU` remains set because the sensor snapshot is otherwise valid. |

Run `bash testing/integration/run_hump_wire.sh normal` and
`bash testing/integration/run_hump_wire.sh sanitize`. Record each result with
its tested revision and evidence when publishing the test run. This host case
does not measure a physical hump or verify behaviour on the Pico.

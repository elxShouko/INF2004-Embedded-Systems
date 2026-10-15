# Buddy 4 — current IMU firmware and tests

Production C is in `app_program/subsystem4/`. Current test cases, host doubles,
runners and results are here, as requested. The template's
`subsystem4_test.c` is only the on-target entry/glue plus retained demo checks.
`testing/subsystem4/` preserves earlier example/archived-branch records; its
113 legacy tests are **not** evidence for the current implementation.

## Build and run

From the repository root, with the existing toolchain and SDK configured:

```sh
bash testing/buddy4/run_host_tests.sh normal
bash testing/buddy4/run_host_tests.sh sanitize
bash testing/buddy4/run_builds.sh
```

The build runner compiles these GNU Make profiles without flashing:

```sh
cd build_make
make -j8
make TEST=1 -j8
make S4_IMU=1 -j8
make TEST=1 S4_IMU=1 -j8
```

Defaults remain SMP=1, CONSOLE=usb_cdc, WIFI=cyw43, NET=none, MQTT=0,
CRASH_REPORT=0. `S4_IMU=0` is the safe default: no IMU pin access. The opt-in
profile produces `_imu.uf2` (or `_test_imu.uf2`) images and changes the stale
object guard. **Do not flash/enable until wiring and calibration are verified.**
The full IMU task is compiled in the enabled profile, not just the host code.
S4_IMU=1 currently requires the template's WIFI=cyw43 SDK clock/time hooks;
it does not use radio/network APIs or imply MQTT is enabled. Linking `-lm`
and the SDK I2C/timeout objects is part of the approved shared Make update.

The host runner uses C99, strict warnings and current source paths; it never
extracts an archived implementation. It tests pure processing and the register
driver, then runs each deterministic RTOS fault-injection case in a fresh
process. The fake kernel checks API use but does not emulate SMP scheduling,
interrupts, priority inheritance, real FIFO capacity or physical bus timing.
Pure cases also compile into TEST=1 and run without touching the live sensor.
Compilation is not an on-target test pass.

## Source map and ownership

| File under `app_program/subsystem4/` | Responsibility |
|---|---|
| `subsystem4.c/.h` | Existing init/start entry points and legacy IF-03/04/05 objects |
| `drv_lsm.c/.h` | Checked register configuration, identity, signed decoding, fresh-data flags |
| `drv_imu.c/.h` | Pico SDK I2C0 transport, GP16/GP17, monotonic timer |
| `imu_core.c/.h` | Calibration, tilt, heading difference, motion, hump state and peak maximum |
| `imu_runtime.c/.h` | Owner task, copied snapshots/events, independent fault latch, speed input |
| `imu_config.h` | Wiring enable, timing and measured magnetic-calibration settings |

All source files remain one level below `app_program/` for template discovery.
The existing `usermain()` all-init-before-any-start order is unchanged. S4's
init creates its objects and dormant IMU task; start only starts it. The
retained monitor runs independently at 18, IMU at 19, within TEAM.md's band.
The monitor source is preserved, not treated as a production safety controller.

Only `imu_runtime_task` accesses the sensor and its processing context.
Snapshot and speed copies use a priority-inheritance kernel mutex; sensor I/O,
math, event sends and delays occur outside the lock. No volatile-based SMP
protocol, new ISR, motor output, Wi-Fi call or kernel-internal edit is added.
Initialization failures prevent S4 start; partial objects remain until reboot
(no automatic retry). The template's shared startup error policy is unchanged.

## Measurement contract and unresolved integration

These provider APIs are **proposals**, not agreed Mission Controller interfaces:

- `imu_runtime_get_status`: copy under a bounded mutex wait; inspect return code,
  `b_enabled`, phase, timestamp and individual validity flags.
- `imu_runtime_get_event`: nonblocking copied status/transition record; caller
  owns the copy. Queue full drops newest and saturating `dropped_events` counts
  loss. Current status and the highest estimate remain separately readable.
- `imu_runtime_wait_fault`: independent, latched flag, never cleared by reads;
  must not be routed through the ordinary telemetry consumer.
- `imu_runtime_set_speed`: proposed calibrated forward-speed input, mm/s,
  timestamp in the `drv_imu_time_ms()` clock, at most 40 ms old. A false validity
  flag revokes the input. No Buddy 2 caller is added or interface assumed.

The core accepts calibrated distance increments directly. Runtime approximates
each increment by latest forward speed times elapsed sample time. Height is
the trapezoidal integral `distance * sin(pitch)` from detected ascent, with the
peak retained. This is an **unvalidated estimator proposal**, not a formula
specified by the professor. It measures estimated sensor-path rise, which is
not automatically the obstacle's true height on a two-axle vehicle. Detection
thresholds can omit the initial rise; acceleration, chassis geometry, mounting,
wheel slip, speed latency and roll can bias it. Never claim centimetre accuracy
until trials with measured humps establish the error and applicability.

Following Week 6 section 6.4 and the user's confirmation, the estimate is
reported and compared with the session maximum **at the peak**, before descent
completion. A missing distance input or detected dynamic-tilt condition during
ascent prevents a valid height report. No distance provider is integrated yet,
so the default firmware does **not** claim a valid measured hump height.
Validity means the estimator's input checks passed, not physical calibration.
The maximum is the highest **valid estimate observed**, not proof of the
mission's true tallest hump if any hump had invalid inputs.

Body axes are X forward, Y left, Z up. Verify both sensor axis mappings; a
different mounting needs an explicitly reviewed transform. Initial stationary,
level calibration estimates acceleration offsets only (not six-face scale or
cross-axis calibration). It resets its sample window on movement and times out
after 10 s. Magnitude and successive-sample checks cannot prove the car is
stationary: the controller/operator must hold it still throughout startup.
No automatic in-motion recalibration or automatic fault recovery occurs.

The LSM303DLHC has no gyroscope. Turning uses shortest-path differences of
tilt-compensated magnetic heading, not gyro yaw rate. `S4_MAG_CALIBRATED=false`
prevents valid heading/turn claims until measured hard-iron offsets and diagonal
scale factors are supplied in `imu_config.h`. This does not correct arbitrary
soft-iron distortion. Test with motors energized; magnetic interference may
make this method unsuitable. No magnetic calibration is fabricated.

Motion is unknown without sufficient evidence; constant-speed motion and rest
cannot be distinguished from acceleration alone. Collision events are threshold
candidates, not proof of contact. Saturated/invalid readings produce faults.

## Timing and fault limits

All numbers below are development settings, **not calibrated values**:

- Accelerometer 100 Hz / high resolution / ±2 g / BDU; magnetometer 75 Hz /
  ±1.3 gauss; owner task nominal 20 ms on the verified 10 ms kernel tick.
  Intermediate accelerometer samples are deliberately skipped, so overrun
  status is expected; software impact detection can miss short impacts.
- Register transaction deadline 2 ms; up to four reads per sample. Runtime
  rejects processing work taking 20 ms or more and samples over 100 ms apart.
  Tick rounding and preemption still add jitter; 100 ms is a stale-data cutoff,
  **not** an acceptable control-loop jitter specification. Measure actual
  worst-case latency, establish the team jitter/deadline limits, then tune.
- Mutex timeout 10 ms; copy-only hold time. Normal event sends never wait.
- 100 calibration samples, ascent 6°, level band 2°, impact 1.8 g, turn 30°/s,
  dynamic magnitude tolerance 0.15 g, roll gate 0.35 rad and hump timeout 10 s.
  `imu_core_defaults()` and the checks are the explicit tuning locations.

The inspected SDK I2C implementation includes reset/FIFO waits outside timeout
checks. Therefore its timeout API is **not a proven hard worst-case bound**
under every hardware fault. The fault consumer must independently monitor
sample freshness; hardware bus-fault and SMP timing tests remain mandatory.
The template clock hook assumes 125 MHz; clock changes require a port review.
Kernel I2C enable conflicts are rejected at compile time for S4_IMU=1; its
default GP8/GP9 driver is not enabled or modified.

Sensor failures, calibration timeout, invalid data, excessive sample gaps,
impact, unfinished humps and runtime failures latch a fault until reboot.
The flag is set before ordinary event publication. Failure of the flag API
ends the IMU task; consumers must treat API errors or stale data as faults.
There is no system-level stop guarantee here: the absent Mission Controller
must implement the reviewed bounded stop request to Buddy 2 and independent
freshness supervision. No motor actuation ownership is bypassed.

## Before hardware acceptance

Use S4-U170–S4-U175 in TEST_LOG.md. Confirm the exact LSM303DLHC breakout,
3.3 V-compatible I/O and pull-ups, common ground, power, GP16/GP17 wiring,
sensor axes, and exclusive I2C0 ownership. Do not infer safe wiring from builds.
Record sensor calibration, speed calibration, sample/drop rates, timing,
thresholds, filtered/noisy traces, ground-truth heights and estimation errors.
Agree numerical acceptance limits before the tests. Retain logs and link
hardware videos through `videos/README.md`; no hardware work was authorized.


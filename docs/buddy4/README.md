# Buddy 4 — current integrated IMU firmware

The Tribunal 1 minimum-bit update is documented in
[`DATA_REPRESENTATION.md`](DATA_REPRESENTATION.md). The event queue now carries
a 32-byte explicit mask/shift representation while the existing public runtime
API, mutex-protected snapshot and independent latched fault flag are preserved.
Numeric measurement widths remain unchanged until calibrated ranges and
precision are agreed.

Production code: [`app_program/subsystem4/`](../../app_program/subsystem4/).
Current tests and results: [`testing/subsystem4/`](../../testing/subsystem4/TEST_LOG.md).
The [older standalone guide](HISTORICAL_STANDALONE.md) documents a different
implementation and 113 archived tests. The current host suite has 82 cases.

## Implementation

`imu_core.c` computes stationary calibration, pitch/roll, magnetic heading,
motion classification, hump states and a provisional maximum at the peak,
following Week 6. `imu_guard.c` decides whether height is trustworthy.
`imu_link.c` reads Buddy 2's signed wheel status. `imu_runtime.c` owns the
sensor/core task, status snapshots, event queue and latched fault flag.
`imu_packed.c` packs the bounded state, classification, validity, event and
height-rejection fields with explicit masks for snapshots and queue messages.
`drv_lsm.c` and `drv_imu.c` own the LSM303DLHC registers and board I2C.
`subsystem4_test.c` is the template's TEST=1 hook. Make discovers the `.c`
files at this depth. The LSM303DLHC has no gyro: turn rate is a difference
of magnetic headings and is invalid when heading data/calibration is absent.

## Height validity

The ascent estimator integrates `distance_mm * sin(pitch)`. This estimates
sensor-path rise; it is not yet validated as physical hump height. Flat
acceleration and braking can imitate tilt while magnitude stays close to 1 g.
A height requires a stable level forward approach, fresh signed wheel status,
straight-wheel targets/measurements, an asserted traction trust input, a
measured motion envelope and persistent ascent. Missing/stale motion, speed
changes beyond calibrated limits, turns, reverse, slip or explicit revocation
invalidates the whole candidate. Reverse speed is never converted to positive
hump distance. The state machine still reports rejected ascent/peak/descent;
state events alone do not imply a valid height.

Week 6's peak update is visible immediately with `b_peak_provisional=true`.
If evidence fails before persistent level closure, an
`IMU_EVENT_HEIGHT_INVALID` retracts that candidate and restores the last
completed valid maximum. Consumers must honor validity, provisional status,
retraction and `height_reasons` bits.

`imu_config.h` disables heading and calibrated height separately. It leaves
the height envelope at zero. Physical trials in
[`VALIDATION.md`](VALIDATION.md) must determine
allowable acceleration, speed drift, wheel mismatch, minimum speed and dwell
times. The host suite uses a labelled synthetic envelope only. Until measured
settings are entered, `b_highest_valid` remains false. Existing 6° ascent,
2° level and 0.15 g magnitude settings are development choices and require
vehicle review; the braking fix does not increase these thresholds.

## Motion and controller contract

`imu_link_read` polls the existing provisional Buddy 2 read-only status and
clock APIs in task context. Wheel speeds and targets are signed mm/s. Buddy 2
provides sample and publication timestamps in kernel operating time
(microseconds, currently 10 ms resolution). Age is checked in that source
clock and read latency is added before the 40 ms freshness limit. Source
timestamp conversion to modulo milliseconds is used only for speed changes;
it is never compared directly with the Pico hardware-timer timestamp. The
distance increment approximates mean wheel speed over the IMU interval, not
encoder position difference. Its slip and accuracy limits need hardware data.

The shared Vehicle Controller now reads the S4 fault latch and status and
requests Buddy 2 emergency stop on a detected fault. It does not assert
traction trust or consume ordinary hump events. Its owner should call
`imu_runtime_set_motion_trust(true)` only after agreeing an independent
traction-confidence policy, then revoke with `false` on slip, unavailable
feedback or mode change. Equal wheel speeds cannot rule out common slip.
Trust defaults false. Revocation advances an epoch, so even a revoke and reassert
between IMU samples invalidates the candidate. A concurrent revoke can take
up to the next 20 ms sample to retract a provisional peak. The former scalar
`imu_runtime_set_speed()` cannot establish straight, nonslip motion: a valid
call returns `E_PAR` and revokes trust; `b_valid=false` revokes explicitly.
Buddy 4 sends no motor command. The proposed trust policy and hardware
acceptance gates are in [`VALIDATION.md`](VALIDATION.md). Fault-to-motor-stop
latency and ordinary hump-event consumption still need integration validation.

Buddy 1's post-tribunal binary telemetry allocates 12 bits for a highest-hump
value in whole millimetres (0–4095). Buddy 4 keeps its float-mm estimate and
validity flags; the shared controller rejects an out-of-range value before
converting it to the wire integer. It must not clamp or silently truncate
4095.9 mm to 4095 mm. The controller already sets Buddy 1's named
`S1_FAULT_IMU` bit from the S4 fault latch; Buddy 4 does not write the same
shared bit a second time. INT-86 records this handoff boundary.

## Build and evidence

From the repository root:

```sh
bash testing/subsystem4/host/run_host_tests.sh normal
bash testing/subsystem4/host/run_host_tests.sh sanitize
bash testing/subsystem4/host/run_builds.sh
```

The build runner compiles the root README's GNU Make profiles: default,
`TEST=1`, `S4_IMU=1`, and both. It does not flash hardware. See the
[test log](../../testing/subsystem4/TEST_LOG.md) for IDs, expected outcomes,
working-tree evidence and hardware gaps. The professor's Project Write-up
and Briefing, submitted Week 6 Design Review, Barr-C:2018 and relevant
hardware documents are in `reference_docs/`.

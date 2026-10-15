# Buddy 4 physical acceptance record — pending

No physical IMU/terrain trial has been run for this revision. Host simulation
and firmware compilation establish software behaviour only. Confirm the exact
LSM303DLHC breakout, voltage, supply, pull-ups, axis mounting and GP16/GP17
wiring against the datasheets before power-up.

## Enablement gates

1. With power removed, identify and photograph the fitted sensor and Robo
   Pico, then confirm part marking, 3.3 V logic compatibility, supply and
   pull-ups. Check continuity from the selected connector's SDA/SCL pins to
   GP16/GP17, common ground, and X-forward/Y-left/Z-up body mounting. Source
   assignment and I2C0 ownership are statically checked; wiring is not.
   Keep `S4_IMU_ENABLE=0` until this record and a stationary bus check pass.
2. Calibrate heading independently. Capture raw field sweeps with motors off
   and on, fit hard-iron offsets and axis scales, then compare heading and
   heading-difference rate with known angles/turns. Record motor interference,
   bias and error. Enable `S4_MAG_CALIBRATED` only within agreed measured
   tolerances. LSM303DLHC has no gyroscope.
3. Calibrate hump height independently after wheel distance calibration.
   Use known heights at different lengths/slopes and repeated controlled
   speeds. Include flat acceleration, braking, accel-then-brake and wheel-slip
   passes. Measure bias, repeatability, absolute/relative error and false
   positives. Set acceleration/speed/wheel/dwell limits from these data and
   verify on held-out passes before enabling `S4_HEIGHT_CALIBRATED`.

## Traction trust — proposed agreement, not yet enabled

The Vehicle/Mission Controller is the proposed sole caller allowed to assert
`imu_runtime_set_motion_trust(true)`, after the team has validated an
independent basis for ground travel on the current surface and mode. The
controller already reads the S4 fault latch/status, but does not assert
traction trust. Matching encoder speeds and targets are not sufficient:
both wheels can slip together. There is no independent slip-confidence
producer in the inspected code, so trust remains false.

The controller should revoke trust (`false`) on suspected slip, stale or
unavailable motion/traction evidence, reverse or turn request, mode change,
obstacle manoeuvre, stop, fault and loss of the validated surface. Record
the positive assertion criterion, owner approval, reassertion criterion,
notification delay and tested operating envelope before implementing it.
Revocation advances an epoch, including when trust is reasserted between
IMU samples. Buddy 2 remains the only drive-motor owner.

## Software fault evidence and physical follow-up

S4-U176 rejects a flat 0.2 g braking sequence that the old core had counted
as 11.187815 mm; S4-U205 tests flat acceleration followed by braking after
a valid 10 mm synthetic hump. These host fixtures prove only their chosen
inputs and synthetic guard envelope. The original reported 2.313 mm input
trace was unavailable. S4-U202/203 check that the SDK wrapper passes its
2 ms timeout/deadline and propagates read/write failures. S4-U155–159
exercise acquisition failure and fault delivery, including a full ordinary
event queue with a separately readable fault flag. Stale motion, reverse,
turn and trust-revocation rejection are covered by S4-U180–184 and U190–199.
None of these software checks establish physical stuck-bus timing, actual
motor-stop latency or hump-height accuracy. Measure these on target as
S4-U170–175 and U200–201 before acceptance.

Use the stable IDs in `testing/subsystem4/TEST_LOG.md`. Save raw IMU, encoder,
status/event and fault traces under `testing/subsystem4/logs/`; record video
links in `videos/README.md`. Each pass needs hardware ID, wheel calibration,
speed, surface, measured hump height/length/slope and instrument uncertainty,
estimated height, peak time, validity bits and motor state.

| ID | Trial | Required evidence |
|---|---|---|
| S4-U170 | Wiring and supply | Part identity, common ground, voltage, I2C pull-ups, pins and axes. |
| S4-U171 | Stationary and known-angle fixtures | Bias, noise, tilt error and moved-start rejection. |
| S4-U172 | Known heights with varied lengths/slopes and repeated controlled speeds | Bias, height error, repeatability, false validity and latency; measured guard limits. |
| S4-U173 | Motor on/off and turns | Heading/rate error and magnetic validity under motor interference. |
| S4-U174 | Disconnect/stuck bus, queue pressure and loaded task | Fault notification, jitter and controller-to-Buddy-2 stop latency. |
| S4-U175 | Restrained impact and rough terrain | False/missed impact rate, latch and stop behaviour. |
| S4-U200 | Flat acceleration, braking and accel-then-brake | Zero valid humps over repeated speeds; keep full traces. |
| S4-U201 | Slip, reverse and in-place turns | Zero valid heights and timely trust revocation. |

For each hump condition calculate bias (mean estimated minus measured height),
mean absolute error, maximum absolute error and sample standard deviation.
Agree numerical acceptance limits before enabling `S4_HEIGHT_CALIBRATED`; use
separate passes for fitting and evaluation. Determine whether sensor-path rise
systematically differs from actual height on long/asymmetric shapes. Record
all run results, including failures, in the test log.

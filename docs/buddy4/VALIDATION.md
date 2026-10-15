# Buddy 4 physical acceptance record — pending

No physical IMU/terrain trial has been run for this revision. Host simulation
and firmware compilation establish software behaviour only. Confirm the exact
LSM303DLHC breakout, voltage, supply, pull-ups, axis mounting and GP16/GP17
wiring against the datasheets before power-up.

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

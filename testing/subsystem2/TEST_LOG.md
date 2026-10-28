# Subsystem 2 — test log

**Subsystem:** Motion Control + retained producer example  ·  **Student ID:** <student ID>  ·  **Code:** `app_program/subsystem2/`

See `testing/README.md` for how to run tests and record results.

## Test cases

Define a test here before running it. Test IDs never change.

S2-U03–U97 were imported retrospectively during the approved 2026-10-14 cleanup.
This does not claim the official definitions existed before historical runs.
All earlier uncommitted results below remain preliminary.

| ID | What is tested | Requirement / interface | Method | Steps / how | Expected result |
|---|---|---|---|---|---|
| S2-U01 | sample values stay within 0..100 | | auto | `subsystem2_test.c` | `[TEST] S2-U01 PASS` |
| S2-U02 | free slots and free blocks within pool size | IF-02 | auto | `subsystem2_test.c` | `[TEST] S2-U02 PASS` |
| S2-U03 | reset and zero configuration remain unconfigured | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_reset; zero fixture`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U03 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U04 | null arguments are rejected | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `NULL init/validate checks`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U04 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U05 | both wheels require nonzero resolution and circumference | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_wheels`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U05 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U06 | unknown and invalid polarities are rejected | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_polarity`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U06 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U07 | zero track and nonpositive speed limits are rejected | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_limits`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U07 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U08 | calibration evidence and enum states are checked | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_evidence`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U08 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U09 | asymmetric wheels and independent polarities are supported | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `asymmetric fixture validation`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U09 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U10 | representation boundaries impose no invented calibration | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_bounds`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U10 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U11 | validation preserves every field on all nonnull paths | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_unchanged`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U11 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U12 | unusual invalid enum values are rejected in every field | M1A configuration | auto / pure | [subsystem2_test.c](../../app_program/subsystem2/subsystem2_test.c): `motion_config_test_enums`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U12 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U13 | quadrature directions, reversal and repeated states | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_quad_cycles`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U13 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U14 | invalid jumps resync; diagnostics saturate | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_quad_invalid`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U14 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U15 | decoder init, null, polarity and state validation | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_quad_args`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U15 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U16 | signed count limits latch until explicit reset | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_quad_limits`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U16 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U17 | independent decoders apply polarity exactly once | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_quad_pair`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U17 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U18 | all sixteen state transitions match the convention | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_quad_table`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U18 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U19 | distance, signed speed and actual variable elapsed time | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_distance`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U19 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U20 | independent left/right wheel scales | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_pair`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U20 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U21 | fractional distance, reversal and repeatability | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_rounding; motion_foundation_est_walk`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U21 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U22 | zero/backward/future/stale timing and fresh zero speed | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_time`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U22 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U23 | count, product, position and speed overflow rejection | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_limits; motion_foundation_est_extremes`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U23 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U24 | source generation and explicit odometry baseline reset | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_reset`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U24 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U25 | invalid/unverified calibration and null estimation | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_config`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U25 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U26 | zero error and signed proportional response | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_response`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U26 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U27 | integral limits, variable elapsed time and reversal | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_integral`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U27 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U28 | output saturation, anti-windup and recovery | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_windup`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U28 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U29 | derivative on measurement avoids target kick | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_deriv`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U29 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U30 | reset, idle, stop and invalid timing clear dynamics | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_reset`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U30 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U31 | invalid gains, bounds, confirmation and null arguments | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_config`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U31 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U32 | nonfinite arithmetic disables output safely | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_limits`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U32 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U33 | independent PID state for both wheels | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_pair`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U33 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U34 | caller edits do not alter copied calibration | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_snapshots`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U34 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U35 | decoder to estimator to PID preserves vehicle sign | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pipeline`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U35 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U36 | pure-I large steps reach both output limits repeatedly | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_crossing`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U36 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U37 | integral crosses saturation and recovers from overspeed | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_unwind`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U37 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U38 | null updates invalidate cached speed and retain baseline | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_null_cache`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U38 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U39 | nonzero fractional speeds and exact timing boundaries | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_est_speed_edges`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U39 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U40 | all PID bounds reject nonfinite and invalid values | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_bad_bounds`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U40 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U41 | derivative, integral and output overflow disable PID | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_nonfinite`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U41 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U42 | asymmetric output headroom includes P/D and I bounds | M1B–M1D foundation | auto / pure | [motion_foundation_test.c](../../app_program/subsystem2/motion_foundation_test.c): `motion_foundation_pid_headroom`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U42 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U43 | Configuration, frequency ceil, limits, invalid/unknown candidates | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_validate`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U43 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U44 | Full/half duty, finite clamping, quantized zero, downward duty cap | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_effort`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U44 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U45 | Independent motor polarity/direction selection | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_polarity`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U45 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U46 | Brake/coast truth-table and Â±zero behaviour | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_zero`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U46 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U47 | Pure initialization, explicit preparation, copied settings | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_startup`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U47 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U48 | Both reversal directions, exact guard, mandatory confirmation | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_reversal`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U48 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U49 | Zero/brake/coast cannot bypass direction history | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_bypass`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U49 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U50 | Pending effort replacement and stop cancellation | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_pending`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U50 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U51 | Null APIs/callbacks, invalid wheel/stop mode, both-wheel fault stop | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_arguments`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U51 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U52 | NaN/Â±Inf input, latched fault, refusal to resume | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_nonfinite`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U52 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U53 | Partial prepare/apply failure and unconfirmed stop | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_faults`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U53 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U54 | Timestamp rollback/wrap fault behaviour | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_clocks`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U54 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U55 | One wheel reversing while the other runs/stops independently | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_independent`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U55 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U56 | Disarm/re-arm, preserved history and fault latch | M2 motor safety | auto / injected or fake output | [test_motor.c](../../app_program/subsystem2/test_motor.c): `test_motor_disarm`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U56 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U57 | Baseline and complete positive quadrature cycle | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_capture`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U57 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U58 | Independent polarity and stationary observation timestamps | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_pair`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U58 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U59 | Invalid transition latches unhealthy | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_invalid`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U59 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U60 | Null, generation and calibration rejection | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_args`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U60 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U61 | Count overflow latches without wrapping | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_limit`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U61 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U62 | Copied mailbox, capacity and terminal acknowledgement gate | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_bus`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U62 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U63 | Pending-command cancellation despite occupied slot | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_cancel`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U63 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U64 | Signed speed/distance, independent loop and explicit brake | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_loop`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U64 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U65 | Stale feedback disarms both outputs | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_stale`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U65 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U66 | Generation reset and null-path rejection | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_reset`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U66 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U67 | Negative overspeed fails closed | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_overspeed`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U67 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U68 | Motor polarity, timing, PID and nonfinite-config consistency | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_loop_bad_config`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U68 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U69 | Failed output stops both; failed brake remains unconfirmed | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_output_error`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U69 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U70 | Zero crossing, reversal guard and count-stable dwell | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_loop_reversal`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U70 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U71 | Relative forward targets and independent wheel arrival | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_plan_forward`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U71 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U72 | Reverse targets and wrong-way rejection | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_plan_reverse`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U72 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U73 | Leading-wheel reduction, integer deceleration cap, overshoot | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_plan_profile`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U73 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U74 | Uninitialised planner, enum, range, future and zero-life rejection | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_plan_invalid`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U74 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U75 | Position/timestamp overflow and INT32_MIN distance | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_plan_overflow`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U75 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U76 | Delayed DONE, durable result, exact acknowledgement and next ID | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_run_travel`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U76 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U77 | Reverse executive completion | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_run_reverse`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U77 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U78 | Cancellation brake/result and replay rejection | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_run_cancel`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U78 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U79 | Continuous refresh, replayed refresh rejection and exact expiry | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_run_expire`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U79 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U80 | Finite exact timeout disarms | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_run_timeout`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U80 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U81 | Emergency latches, acknowledgement cannot re-enable | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_run_faults`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U81 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U82 | Repeated stops cannot extend stopping deadline | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_run_stop_deadline`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U82 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U83 | Left/right 90/180-degree target signs and arc values | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_turn_goals`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U83 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U84 | Unequal calibrated turn arcs and normalized leading-wheel correction | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_turn_scale`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U84 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U85 | Turn overflow, zero scale and unrepresentably small arc rejection | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_turn_overflow`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U85 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U86 | Both turn wheel targets and standstill required | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_turn_completion`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U86 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U87 | Post-braking turn drift becomes OVERSHOOT | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_turn_overshoot`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U87 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U88 | Unhealthy capture becomes SENSOR fault | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_encoder_fault`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U88 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U89 | Failed status publication cannot reopen admission prematurely | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_bus_inflight`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U89 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U90 | Early/duplicate wakes skip PID; backward time faults | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_sample_time`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U90 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U91 | No-progress timeout brakes despite repeated nonzero requests | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_progress_timeout`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U91 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U92 | Caller edits cannot change copied control calibration | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_config_copy`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U92 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U93 | New publication cannot refresh stale feedback | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_freshness`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U93 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U94 | Request acceptance/rejection metadata and brake confirmation | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_admission_result`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U94 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U95 | Emergency cause and earlier durable terminal coexist | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_durable_fault`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U95 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U96 | Moving wheel cannot hold reversal indefinitely | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_reversal_timeout`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U96 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |
| S2-U97 | Injected GPIO→decoder→estimator→executive→PID→fake PWM and fault stop | M3–M7 motion | auto / injected or fake output | [test_drive.c](../../app_program/subsystem2/test_drive.c): `test_motion_end_to_end`; host runner or native TEST=1 worker | Every assertion true; `[TEST] S2-U97 PASS`. Expected rejection/fault paths must reject/fault as asserted; no physical actuation. |

Added auto groups also use custom report helpers; see execution boundaries below.

*Method:* **auto** = `TEST_CHECK` in code, result printed on the console;
**manual** = a person observes the result (LED, scope, meter, phone app…).

## Results

**The original example row below is retained verbatim as a PLACEHOLDER ONLY.**
`YYYY-MM-DD`, `abc1234`, `PASS / FAIL` and its example path are not real execution
evidence. Do not count this row as a pass.

Add a new row for every run. Never edit or delete old rows.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| YYYY-MM-DD | S2-U01 | `abc1234` | | PASS / FAIL | `logs/YYYY-MM-DD_….txt` | |

## Revision metadata and interpretation

This cleanup imported existing evidence; it did not execute tests or compilers.
All archived development results used uncommitted working trees. A historical
PASS is **PRELIMINARY**, not formal commit-verified acceptance. No commit was
performed. Commit-before-testing cannot be satisfied retrospectively; a future
formal run needs an authorised committed revision and newly saved original logs.

| Revision key | Available information | Limitation |
|---|---|---|
| M2-WT | M2 narrative reports an uncommitted working tree | Exact base commit and full tested source snapshot are not recorded with the raw M2 logs; the later storage commit is not evidence of the tested revision |
| M3-WT through M7-WT; STRICT-WT | Continuous-development handoff reports base HEAD `9e40a56d55955291c54957c459e02af1480bc31e`, branch `buddy2-Motor-and-encoder-control`, plus uncommitted files | Exact intermediate snapshots and run clock times are unavailable; base HEAD alone is not the tested code |
| FINAL-WT | Same base HEAD plus uncommitted M3–M7 changes; archived ownership report records source/header/script SHA256 | Preliminary working-tree evidence, not a clean commit; ownership report discloses a later token-preserving line wrap |
| Earlier foundation narrative | Reports base `2886fa8a0118185674f59964ab832d7c4e3ceabb` plus dirty changes | Separate original foundation logs are unavailable in this archive; standalone run rows are not fabricated from prose |

Development documents report the date 2026-10-14. Exact start/end times and
original human/operator identity were not recorded. Tester attribution below is
therefore `Unrecorded (automated run)`, not an invented person. Archive filenames
are not substitutes for revision metadata. See the
[evidence manifest](logs/2026-10-14_evidence_manifest.md) for original paths,
sizes, hashes and source preservation. A later code change is not certified by
these earlier logs.

## Execution boundaries

| Activity | Actual status | Evidence and limitations |
|---|---|---|
| S2-U03–U97, final default host profile | 95 PRELIMINARY HOST PASS | [Original output](logs/2026-10-14_verify_final_default.txt); one result per ID |
| S2-U03–U97, final O2/fshort-enums host profile | 95 PRELIMINARY HOST PASS | [Original output](logs/2026-10-14_verify_final_optimized.txt); one result per ID |
| S2-U01 | NOT RUN / no archived execution evidence | Inherited target sample-value check, excluded from the 95-group host entry point |
| S2-U02 | NOT RUN / no archived execution evidence | Inherited live semaphore/pool check, excluded from the 95-group host entry point |
| Native TEST=1 execution, S2-U01–U97 | NOT RUN / no target console evidence | Registration and ARM compilation are not execution |
| ARM compilation | Preliminary compiler outcomes below | Object compilation only; not tests, full linking or hardware validation |
| Integrated firmware linking | No successful full link recorded here | Known SDK 1.5.1 versus required 2.2.0 compatibility block |
| Encoder IRQ, motor/PID hardware, travel/turn accuracy, SMP timing and emergency latency | NOT RUN / NOT HARDWARE-VALIDATED | Synthetic fixtures are not calibration or electrical/timing safety evidence |

TEST=1 remains registered through subsystem2_test.c. Pure suites run on its
static-stack worker using TEST_CHECK adapters and custom result helpers. No C/H
file was moved here. S2-U57–U97 derive from a fixed function array: do not reorder
existing entries or reuse IDs. No new unit-test ID was allocated during cleanup.

## Historical host-run summary

Each entry refers to an available original log, not a cleanup execution. None
contains S2-U01/U02. Each recorded ID has its own ledger row below.

| Run/profile | Revision key | IDs recorded | PASS / FAIL | Original evidence |
|---|---|---|---|---|
| m2/default | M2-WT | S2-U03–U56 | 54 / 0 (preliminary) | [2026-10-14_m2_host_default.txt](logs/2026-10-14_m2_host_default.txt) |
| m2/optimized | M2-WT | S2-U03–U56 | 54 / 0 (preliminary) | [2026-10-14_m2_host_optimized.txt](logs/2026-10-14_m2_host_optimized.txt) |
| M3/default | M3-WT | S2-U03–U61 | 59 / 0 (preliminary) | [2026-10-14_verify_M3_default.txt](logs/2026-10-14_verify_M3_default.txt) |
| M3/optimized | M3-WT | S2-U03–U61 | 59 / 0 (preliminary) | [2026-10-14_verify_M3_optimized.txt](logs/2026-10-14_verify_M3_optimized.txt) |
| M4/default | M4-WT | S2-U03–U63 | 61 / 0 (preliminary) | [2026-10-14_verify_M4_default.txt](logs/2026-10-14_verify_M4_default.txt) |
| M4/optimized | M4-WT | S2-U03–U63 | 61 / 0 (preliminary) | [2026-10-14_verify_M4_optimized.txt](logs/2026-10-14_verify_M4_optimized.txt) |
| M5/default | M5-WT | S2-U03–U70 | 68 / 0 (preliminary) | [2026-10-14_verify_M5_default.txt](logs/2026-10-14_verify_M5_default.txt) |
| M5/optimized | M5-WT | S2-U03–U70 | 68 / 0 (preliminary) | [2026-10-14_verify_M5_optimized.txt](logs/2026-10-14_verify_M5_optimized.txt) |
| M6/default | M6-WT | S2-U03–U82 | 80 / 0 (preliminary) | [2026-10-14_verify_M6_default.txt](logs/2026-10-14_verify_M6_default.txt) |
| M6/optimized | M6-WT | S2-U03–U82 | 80 / 0 (preliminary) | [2026-10-14_verify_M6_optimized.txt](logs/2026-10-14_verify_M6_optimized.txt) |
| M7/default | M7-WT | S2-U03–U90 | 88 / 0 (preliminary) | [2026-10-14_verify_M7_default.txt](logs/2026-10-14_verify_M7_default.txt) |
| M7/optimized | M7-WT | S2-U03–U90 | 88 / 0 (preliminary) | [2026-10-14_verify_M7_optimized.txt](logs/2026-10-14_verify_M7_optimized.txt) |
| final/default | FINAL-WT | S2-U03–U97 | 95 / 0 (preliminary) | [2026-10-14_verify_final_default.txt](logs/2026-10-14_verify_final_default.txt) |
| final/optimized | FINAL-WT | S2-U03–U97 | 95 / 0 (preliminary) | [2026-10-14_verify_final_optimized.txt](logs/2026-10-14_verify_final_optimized.txt) |

The two M2 host logs are byte-identical and retained separately. Their compiler
profile attribution comes from the M2 narrative; raw output contains no differing
compiler invocation or exit-code marker. Later logs include commands and EXITs.

<details>
<summary>Complete preliminary host-result ledger: 1,010 saved per-ID outcomes</summary>

| Date reported | Test ID | Tested revision key | Tester | Actual result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-14 | S2-U03 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/default](logs/2026-10-14_m2_host_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M2-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [m2/optimized](logs/2026-10-14_m2_host_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/default](logs/2026-10-14_verify_M3_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M3-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M3/optimized](logs/2026-10-14_verify_M3_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/default](logs/2026-10-14_verify_M4_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M4-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M4/optimized](logs/2026-10-14_verify_M4_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/default](logs/2026-10-14_verify_M5_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | M5-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M5/optimized](logs/2026-10-14_verify_M5_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U71 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U72 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U73 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U74 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U75 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U76 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U77 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U78 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U79 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U80 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U81 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U82 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/default](logs/2026-10-14_verify_M6_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U71 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U72 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U73 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U74 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U75 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U76 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U77 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U78 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U79 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U80 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U81 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U82 | M6-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M6/optimized](logs/2026-10-14_verify_M6_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U71 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U72 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U73 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U74 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U75 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U76 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U77 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U78 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U79 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U80 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U81 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U82 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U83 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U84 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U85 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U86 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U87 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U88 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U89 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U90 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/default](logs/2026-10-14_verify_M7_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U71 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U72 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U73 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U74 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U75 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U76 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U77 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U78 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U79 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U80 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U81 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U82 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U83 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U84 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U85 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U86 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U87 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U88 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U89 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U90 | M7-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [M7/optimized](logs/2026-10-14_verify_M7_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U71 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U72 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U73 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U74 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U75 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U76 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U77 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U78 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U79 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U80 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U81 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U82 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U83 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U84 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U85 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U86 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U87 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U88 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U89 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U90 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U91 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U92 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U93 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U94 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U95 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U96 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U97 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/default](logs/2026-10-14_verify_final_default.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U03 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U04 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U05 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U06 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U07 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U08 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U09 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U10 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U11 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U12 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U13 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U14 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U15 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U16 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U17 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U18 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U19 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U20 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U21 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U22 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U23 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U24 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U25 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U26 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U27 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U28 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U29 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U30 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U31 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U32 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U33 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U34 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U35 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U36 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U37 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U38 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U39 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U40 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U41 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U42 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U43 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U44 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U45 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U46 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U47 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U48 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U49 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U50 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U51 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U52 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U53 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U54 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U55 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U56 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U57 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U58 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U59 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U60 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U61 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U62 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U63 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U64 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U65 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U66 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U67 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U68 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U69 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U70 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U71 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U72 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U73 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U74 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U75 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U76 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U77 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U78 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U79 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U80 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U81 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U82 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U83 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U84 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U85 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U86 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U87 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U88 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U89 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U90 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U91 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U92 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U93 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U94 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U95 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U96 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |
| 2026-10-14 | S2-U97 | FINAL-WT (dirty) | Unrecorded (automated run) | PRELIMINARY HOST PASS | [final/optimized](logs/2026-10-14_verify_final_optimized.txt) | Imported original outcome; not commit-verified or on-target |

</details>

## ARM compilation history — not unit-test or hardware results

All compiler EXIT outcomes are preserved, including failed attempts. Later
successes do not erase failures. Historical input names/profiles are retained;
exact fix commits are unknown and are not fabricated.

| Evidence | Compiler records | Exit 0 / nonzero | Interpretation |
|---|---|---|---|
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 60 | 58 / 2 | Preliminary M2-WT; objects only |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 76 | 76 / 0 | Preliminary M7-WT; objects only |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 76 | 76 / 0 | Preliminary STRICT-WT; objects only |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 76 | 76 / 0 | Preliminary FINAL-WT; objects only |

The M2 archive retains two failed attempts: missing DEVCNF_USE_IIC declaration
and strict inherited-kernel-header diagnostics. Later successful records exist
in the same original archive. The exact fix commit is unknown. Historical
input drv_motor_rp2040.c does not require restoring a duplicate of drv_rp2040.c.

<details>
<summary>Complete compiler-attempt ledger, including retained failures</summary>

| Evidence | Command line | Historical input | SMP | APP_TEST | Exit | Actual outcome | Revision |
|---|---:|---|---:|---:|---:|---|---|
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 1 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 3 | `drv_motor_rp2040.c` | 1 | 0 | 1 | COMPILE FAIL — retained | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 10 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 12 | `drv_motor_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 14 | `drv_motor_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 16 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 18 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 20 | `drv_motor_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 22 | `drv_motor_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 24 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 28 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 30 | `drv_motor_rp2040.c` | 1 | 0 | 1 | COMPILE FAIL — retained | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 47 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 50 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 53 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 56 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 59 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 62 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 65 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 68 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 71 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 74 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 77 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 80 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 83 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 86 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 89 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 92 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 97 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 100 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 103 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 106 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 109 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 112 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 115 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 118 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 121 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 124 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 127 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 130 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 133 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 136 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 139 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 142 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 147 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 150 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 153 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 156 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 159 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 162 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 165 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 168 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 171 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 174 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 177 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 180 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 183 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 186 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 189 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_m2_arm_compile.txt](logs/2026-10-14_m2_arm_compile.txt) | 192 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M2-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 1 | `drv_encoder.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 3 | `drv_motor.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 5 | `drv_rp2040.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 7 | `enc_capture.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 9 | `motion_bus.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 11 | `motion_config.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 13 | `motion_estimator.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 15 | `motion_foundation_test.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 17 | `motion_pid.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 19 | `motion_quad.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 21 | `motion_run.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 23 | `move_plan.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 25 | `subsystem2.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 27 | `subsystem2_test.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 29 | `task_motion.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 31 | `task_producer.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 33 | `test_motion.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 35 | `test_motor.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 37 | `wheel_loop.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 39 | `drv_encoder.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 41 | `drv_motor.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 43 | `drv_rp2040.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 45 | `enc_capture.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 47 | `motion_bus.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 49 | `motion_config.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 51 | `motion_estimator.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 53 | `motion_foundation_test.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 55 | `motion_pid.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 57 | `motion_quad.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 59 | `motion_run.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 61 | `move_plan.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 63 | `subsystem2.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 65 | `subsystem2_test.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 67 | `task_motion.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 69 | `task_producer.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 71 | `test_motion.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 73 | `test_motor.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 75 | `wheel_loop.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 77 | `drv_encoder.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 79 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 81 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 83 | `enc_capture.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 85 | `motion_bus.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 87 | `motion_config.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 89 | `motion_estimator.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 91 | `motion_foundation_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 93 | `motion_pid.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 95 | `motion_quad.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 97 | `motion_run.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 99 | `move_plan.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 101 | `subsystem2.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 103 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 105 | `task_motion.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 107 | `task_producer.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 109 | `test_motion.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 111 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 113 | `wheel_loop.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 115 | `drv_encoder.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 117 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 119 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 121 | `enc_capture.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 123 | `motion_bus.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 125 | `motion_config.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 127 | `motion_estimator.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 129 | `motion_foundation_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 131 | `motion_pid.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 133 | `motion_quad.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 135 | `motion_run.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 137 | `move_plan.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 139 | `subsystem2.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 141 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 143 | `task_motion.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 145 | `task_producer.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 147 | `test_motion.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 149 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_M7_arm.txt](logs/2026-10-14_verify_M7_arm.txt) | 151 | `wheel_loop.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | M7-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 1 | `drv_encoder.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 3 | `drv_motor.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 5 | `drv_rp2040.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 7 | `enc_capture.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 9 | `motion_bus.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 11 | `motion_config.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 13 | `motion_estimator.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 15 | `motion_foundation_test.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 17 | `motion_pid.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 19 | `motion_quad.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 21 | `motion_run.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 23 | `move_plan.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 25 | `subsystem2.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 27 | `subsystem2_test.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 29 | `task_motion.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 31 | `task_producer.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 33 | `test_drive.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 35 | `test_motor.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 37 | `wheel_loop.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 39 | `drv_encoder.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 41 | `drv_motor.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 43 | `drv_rp2040.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 45 | `enc_capture.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 47 | `motion_bus.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 49 | `motion_config.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 51 | `motion_estimator.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 53 | `motion_foundation_test.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 55 | `motion_pid.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 57 | `motion_quad.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 59 | `motion_run.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 61 | `move_plan.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 63 | `subsystem2.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 65 | `subsystem2_test.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 67 | `task_motion.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 69 | `task_producer.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 71 | `test_drive.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 73 | `test_motor.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 75 | `wheel_loop.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 77 | `drv_encoder.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 79 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 81 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 83 | `enc_capture.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 85 | `motion_bus.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 87 | `motion_config.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 89 | `motion_estimator.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 91 | `motion_foundation_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 93 | `motion_pid.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 95 | `motion_quad.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 97 | `motion_run.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 99 | `move_plan.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 101 | `subsystem2.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 103 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 105 | `task_motion.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 107 | `task_producer.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 109 | `test_drive.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 111 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 113 | `wheel_loop.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 115 | `drv_encoder.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 117 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 119 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 121 | `enc_capture.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 123 | `motion_bus.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 125 | `motion_config.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 127 | `motion_estimator.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 129 | `motion_foundation_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 131 | `motion_pid.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 133 | `motion_quad.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 135 | `motion_run.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 137 | `move_plan.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 139 | `subsystem2.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 141 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 143 | `task_motion.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 145 | `task_producer.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 147 | `test_drive.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 149 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_strict_arm.txt](logs/2026-10-14_verify_strict_arm.txt) | 151 | `wheel_loop.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | STRICT-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 1 | `drv_encoder.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 3 | `drv_motor.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 5 | `drv_rp2040.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 7 | `enc_capture.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 9 | `motion_bus.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 11 | `motion_config.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 13 | `motion_estimator.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 15 | `motion_foundation_test.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 17 | `motion_pid.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 19 | `motion_quad.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 21 | `motion_run.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 23 | `move_plan.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 25 | `subsystem2.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 27 | `subsystem2_test.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 29 | `task_motion.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 31 | `task_producer.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 33 | `test_drive.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 35 | `test_motor.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 37 | `wheel_loop.c` | 0 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 39 | `drv_encoder.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 41 | `drv_motor.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 43 | `drv_rp2040.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 45 | `enc_capture.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 47 | `motion_bus.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 49 | `motion_config.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 51 | `motion_estimator.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 53 | `motion_foundation_test.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 55 | `motion_pid.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 57 | `motion_quad.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 59 | `motion_run.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 61 | `move_plan.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 63 | `subsystem2.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 65 | `subsystem2_test.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 67 | `task_motion.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 69 | `task_producer.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 71 | `test_drive.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 73 | `test_motor.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 75 | `wheel_loop.c` | 0 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 77 | `drv_encoder.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 79 | `drv_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 81 | `drv_rp2040.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 83 | `enc_capture.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 85 | `motion_bus.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 87 | `motion_config.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 89 | `motion_estimator.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 91 | `motion_foundation_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 93 | `motion_pid.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 95 | `motion_quad.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 97 | `motion_run.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 99 | `move_plan.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 101 | `subsystem2.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 103 | `subsystem2_test.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 105 | `task_motion.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 107 | `task_producer.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 109 | `test_drive.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 111 | `test_motor.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 113 | `wheel_loop.c` | 1 | 0 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 115 | `drv_encoder.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 117 | `drv_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 119 | `drv_rp2040.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 121 | `enc_capture.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 123 | `motion_bus.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 125 | `motion_config.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 127 | `motion_estimator.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 129 | `motion_foundation_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 131 | `motion_pid.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 133 | `motion_quad.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 135 | `motion_run.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 137 | `move_plan.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 139 | `subsystem2.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 141 | `subsystem2_test.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 143 | `task_motion.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 145 | `task_producer.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 147 | `test_drive.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 149 | `test_motor.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |
| [2026-10-14_verify_final_arm.txt](logs/2026-10-14_verify_final_arm.txt) | 151 | `wheel_loop.c` | 1 | 1 | 0 | PRELIMINARY COMPILE PASS | FINAL-WT (dirty) |

</details>

The final archive records 19 successful translation-unit compilations in EACH
of SMP=0/1 × APP_TEST=0/1, 76 total. No complete firmware link or target execution
is implied. Ancillary evidence:

- [Compiler stack estimates](logs/2026-10-14_verify_final_stack.txt): individual
  function frames, not measured target margin or a full call-chain bound.
- [Make recipe copy](logs/2026-10-14_m2_make_recipes.txt): the source-folder original
  remains because verify_motion.py reads it.
- [Historical ownership report](logs/2026-10-14_motion_ownership.txt): byte-preserved
  development record; use the manifest to resolve its original relative paths.
- [Foundation narrative](../../docs/buddy2/MOTION_FOUNDATION_VERIFICATION.md)
  and [motor narrative](../../docs/buddy2/MOTOR_DRIVER.md): preserved
  historical records, not substitutes for absent raw evidence.

## Remaining limitations and next formal run

Some older runs lack exact snapshots, times or attribution; missing evidence is
not reconstructed. The unchanged verify_motion.py still writes outputs/logs in
the source folder and overwrites repeated labels. Archived files here are safe;
future output-directory/immutable-name improvements require separate approval.
Shell TEMP/TMP alone cannot override the script's hardcoded values.

Formal compliance needs an authorised committed revision and a subsequent run,
with result rows and logs recorded together. Target and powered tests require
separate authorisation and real console/instrument evidence. No hardware PASS
is recorded here. Never overwrite archives, delete old outcomes, renumber IDs,
or treat compilation as hardware validation.

## Commit-verified host rerun — 2026-10-14

Tested implementation: `50ee11ce345b86ce83559c47c0a2e8be055f6778` on `buddy2-motion-control-pr`.
Tracked files were clean at execution start. This follow-up changes only test
records, leaving the tested implementation unchanged. Both strict C99 profiles
with -fanalyzer passed all 95 host groups without compiler warnings. S2-U01/U02
remain unverified on target; host results do not establish hardware/RTOS safety.
Exact commands, timings and byte-preserved raw logs are in the
[commit-verified evidence](logs/2026-10-14_pr_preflight_manifest.md#commit-verified-host-rerun).
Prior historical results are unchanged.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-14 | S2-U03 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U04 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U05 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U06 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U07 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U08 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U09 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U10 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U11 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U12 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U13 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U14 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U15 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U16 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U17 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U18 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U19 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U20 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U21 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U22 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U23 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U24 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U25 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U26 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U27 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U28 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U29 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U30 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U31 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U32 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U33 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U34 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U35 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U36 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U37 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U38 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U39 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U40 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U41 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U42 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U43 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U44 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U45 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U46 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U47 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U48 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U49 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U50 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U51 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U52 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U53 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U54 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U55 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U56 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U57 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U58 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U59 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U60 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U61 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U62 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U63 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U64 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U65 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U66 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U67 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U68 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U69 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U70 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U71 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U72 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U73 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U74 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U75 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U76 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U77 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U78 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U79 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U80 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U81 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U82 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U83 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U84 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U85 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U86 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U87 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U88 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U89 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U90 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U91 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U92 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U93 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U94 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U95 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U96 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U97 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw default evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-default) | Commit-verified default; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U03 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U04 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U05 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U06 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U07 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U08 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U09 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U10 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U11 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U12 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U13 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U14 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U15 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U16 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U17 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U18 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U19 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U20 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U21 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U22 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U23 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U24 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U25 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U26 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U27 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U28 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U29 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U30 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U31 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U32 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U33 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U34 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U35 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U36 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U37 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U38 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U39 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U40 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U41 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U42 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U43 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U44 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U45 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U46 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U47 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U48 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U49 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U50 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U51 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U52 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U53 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U54 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U55 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U56 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U57 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U58 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U59 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U60 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U61 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U62 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U63 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U64 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U65 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U66 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U67 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U68 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U69 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U70 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U71 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U72 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U73 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U74 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U75 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U76 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U77 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U78 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U79 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U80 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U81 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U82 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U83 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U84 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U85 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U86 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U87 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U88 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U89 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U90 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U91 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U92 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U93 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U94 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U95 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U96 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |
| 2026-10-14 | S2-U97 | `50ee11ce345b86ce83559c47c0a2e8be055f6778` | Codex (automated host run) | HOST PASS | [Raw optimized evidence](logs/2026-10-14_pr_preflight_manifest.md#committed-host-optimized) | Commit-verified optimized; synthetic outputs; no target/hardware validation. |

## Final folder verification — 2026-10-14

Tested commit: `143a3aa2a0a00d5c5038d15f8b97423b5b7ec7f7`; tracked tree clean at execution start.
Run UTC 14:25:36–14:25:49. Production/test code and verification script unchanged.
Per-ID outcomes and byte-preserved raw logs are in the
[organisation verification evidence](logs/2026-10-14_pr_preflight_manifest.md#final-organisation-verification).

| Profile | Coverage | Result | Evidence |
|---|---|---|---|
| Strict C99 default + -fanalyzer | S2-U03–U97 | 95 HOST PASS, zero warnings | [Raw output](logs/2026-10-14_pr_preflight_manifest.md#organisation-host-default) |
| Strict C99 O2/fshort-enums + -fanalyzer | S2-U03–U97 | 95 HOST PASS, zero warnings | [Raw output](logs/2026-10-14_pr_preflight_manifest.md#organisation-host-optimized) |

S2-U01/U02, target execution, complete firmware linking and physical hardware
validation remain unverified. Existing historical records are unchanged.

## Normal-repository finalisation verification — 2026-10-15

Base HEAD: `0482b352c4d30e8e95831ae8ff931c8952dbfeb9`, branch `buddy2-Motor-and-encoder-control`.
The organisation/docs/evidence are uncommitted. New results below are WORKING
TREE verification, not a new commit-verified run. Production/tests/tooling are
unchanged from that base. Existing historical results and failure records are
preserved. Exact commands, source hashes, initial ARM launch failure and retry
are described in [verification manifest](logs/2026-10-15_final_verification.md).
S2-U01/U02 and all target/hardware execution remain unverified.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S2-U03 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U04 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U05 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U06 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U07 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U08 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U09 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U10 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U11 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U12 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U13 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U14 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U15 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U16 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U17 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U18 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U19 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U20 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U21 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U22 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U23 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U24 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U25 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U26 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U27 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U28 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U29 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U30 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U31 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U32 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U33 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U34 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U35 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U36 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U37 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U38 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U39 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U40 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U41 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U42 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U43 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U44 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U45 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U46 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U47 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U48 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U49 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U50 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U51 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U52 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U53 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U54 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U55 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U56 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U57 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U58 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U59 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U60 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U61 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U62 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U63 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U64 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U65 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U66 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U67 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U68 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U69 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U70 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U71 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U72 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U73 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U74 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U75 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U76 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U77 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U78 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U79 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U80 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U81 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U82 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U83 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U84 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U85 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U86 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U87 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U88 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U89 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U90 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U91 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U92 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U93 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U94 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U95 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U96 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U97 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw default output](logs/2026-10-15_finalnormal_host_default.txt) | Strict C99 default, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U03 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U04 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U05 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U06 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U07 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U08 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U09 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U10 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U11 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U12 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U13 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U14 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U15 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U16 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U17 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U18 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U19 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U20 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U21 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U22 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U23 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U24 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U25 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U26 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U27 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U28 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U29 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U30 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U31 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U32 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U33 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U34 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U35 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U36 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U37 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U38 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U39 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U40 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U41 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U42 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U43 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U44 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U45 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U46 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U47 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U48 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U49 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U50 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U51 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U52 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U53 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U54 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U55 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U56 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U57 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U58 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U59 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U60 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U61 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U62 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U63 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U64 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U65 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U66 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U67 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U68 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U69 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U70 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U71 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U72 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U73 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U74 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U75 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U76 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U77 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U78 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U79 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U80 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U81 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U82 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U83 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U84 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U85 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U86 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U87 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U88 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U89 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U90 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U91 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U92 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U93 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U94 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U95 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U96 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |
| 2026-10-15 | S2-U97 | `0482b352c4d30e8e95831ae8ff931c8952dbfeb9` + uncommitted organisation | Codex (automated host run) | HOST PASS / WORKING TREE | [Raw optimized output](logs/2026-10-15_finalnormal_host_optimized.txt) | Strict C99 optimized, -fanalyzer; synthetic outputs; not target/hardware validation. |

| Verification activity | Result | Evidence |
|---|---|---|
| Initial ARM compiler launch in sandbox | BLOCKED: WinError 623 before compiler execution | [Context and empty-log handling](logs/2026-10-15_final_verification.md) |
| ARM SMP=0 TEST=0 | 19 objects compiled, no warnings | [Original compiler output](logs/2026-10-15_finalnormal_arm_strict.txt) |
| ARM SMP=0 TEST=1 | 19 objects compiled, no warnings | [Original compiler output](logs/2026-10-15_finalnormal_arm_strict.txt) |
| ARM SMP=1 TEST=0 | 19 objects compiled, no warnings | [Original compiler output](logs/2026-10-15_finalnormal_arm_strict.txt) |
| ARM SMP=1 TEST=1 | 19 objects compiled, no warnings | [Original compiler output](logs/2026-10-15_finalnormal_arm_strict.txt) |
| Full firmware link / target / physical hardware | NOT RUN; prerequisites unresolved | [Limitations](logs/2026-10-15_final_verification.md) |

## Historical integrated ARM evidence relocated — 2026-10-15

The six previously committed build_make verification logs are now archived
here byte-for-byte. This is evidence organisation only: no compiler, Make
build, host test or hardware test was executed for the relocation. Original
commands, failures, sizes and metadata remain unchanged inside the raw files.
Metadata reports base HEAD 2886fa8a… during historical working-tree verification;
it does not prove a clean committed tested implementation.

| Archived evidence | Historical meaning |
|---|---|
| [2026-10-14_integrated_arm_metadata.txt](logs/2026-10-14_integrated_arm_metadata.txt) | Historical commands, profile, object sizes and ELF-header metadata |
| [2026-10-14_integrated_make_normal_sdk_failure.txt](logs/2026-10-14_integrated_make_normal_sdk_failure.txt) | Normal integrated Make attempt; SDK USB pico.h failure; no link |
| [2026-10-14_integrated_arm_objects_normal.txt](logs/2026-10-14_integrated_arm_objects_normal.txt) | Historical normal-profile foundation object compilation |
| [2026-10-14_integrated_arm_objects_test.txt](logs/2026-10-14_integrated_arm_objects_test.txt) | Historical TEST=1 foundation object compilation |
| [2026-10-14_integrated_make_test_missing_compiler.txt](logs/2026-10-14_integrated_make_test_missing_compiler.txt) | Initial TEST=1 Make attempt; compiler absent from PATH |
| [2026-10-14_integrated_make_test_sdk_failure.txt](logs/2026-10-14_integrated_make_test_sdk_failure.txt) | Retried TEST=1 Make attempt; SDK USB pico.h failure; no link |

Exact original-to-archive paths, byte lengths and SHA256 values are in the
[relocation integrity record](logs/2026-10-15_final_verification.md#integrated-arm-log-relocation--2026-10-15).
Current approval retains root AGENTS.md and reference_docs unchanged. The final
working-tree build_make diff against local main is empty; committed HEAD still
contains the old paths until the user commits the relocation. No Git history
was rewritten. Full linking/target/hardware limitations remain unchanged.

## Post-merge Buddy 2 adapter verification — 2026-10-15

New stable definitions below extend the registry; S2-U01–U97 are unchanged.
U90 additionally checks ACTIVE refreshes, duplicate wakes and the next PID
deadline. Existing READY/backward-time assertions remain. All new cases are
host-only: actual task_motion.c and drv_encoder.c use test_rtos doubles.
Each adapter case runs in a fresh process. No physical GPIO, PWM or IRQ access,
real scheduler, priority inheritance or SMP execution is simulated as proven.
These definitions follow an initial investigative console run, not a claim
that they were registered before that development experiment. That run found
U99/U100/U101 failing because the original init returned E_LIMIT instead of the
injected E_SYS. No separate raw log was captured for that initial experiment.

| ID | Definition / method | Expected result |
|---|---|---|
| S2-U98 | Run actual unconfigured owner entry | Infinite event wait; no GPIO or motor preparation |
| S2-U99 | Inject mutex creation E_SYS | Preserve error, clean only owned objects, API unavailable |
| S2-U100 | Inject wake flag creation E_SYS | Preserve error and delete created mutex |
| S2-U101 | Inject safety flag creation E_SYS | Preserve error and delete created wake/mutex |
| S2-U102 | Inject spinlock initialization failure | Preserve error; delete all created objects |
| S2-U103 | Inject task creation failure | Preserve error; delete flags/mutex |
| S2-U104 | Inject task start failure | Return start error; no motor preparation |
| S2-U105 | ACTIVE refresh wakes at 40/50 ms with 30 ms fixture period | No early PID; first drive at 60 ms, paired output exactly once |
| S2-U106 | Hold command/status mutex, then emergency stop | Ordinary stop returns BUSY; safety post succeeds; braking precedes mutex release |
| S2-U107 | Inject safety flag post error | Emergency API returns ERROR; wake still attempted |
| S2-U108 | Inject wake flag post error | Emergency API returns ERROR; safety latch still posted |
| S2-U109 | Reject encoder reservation | Fault before GPIO setup/motor preparation; no release of foreign reservation |
| S2-U110 | Reject encoder IRQ enable | Release granted reservation once; no motor preparation |
| S2-U111 | Fail GP28 input setup | Release reservation once; never enable motors |
| S2-U112 | Deliver invalid left quadrature jump during ACTIVE | Sensor fault; both outputs braked; encoder released |
| S2-U113 | Fail SMP capture try-lock during callback | Sensor fault; both outputs braked; encoder released |
| S2-U114 | Fail operating-time read during ACTIVE | Owner exits faulted; outputs braked and encoder released |
| S2-U115 | Fail event wait during ACTIVE | Owner exits faulted; outputs braked and encoder released |
| S2-U116 | Query measurement older than allowed age | Speed validity/standstill false; publication does not refresh sample |
| S2-U117 | Omit reserve hook | Configure/open rejected without GPIO operations |
| S2-U118 | GP7 has foreign PWM mux | Refuse acquisition before reserve/GPIO/motor setup |
| S2-U119 | Deliver one left encoder edge through registered sink | Left +100 um/+10 mm/s; right unchanged; fixture units only |
| S2-U120 | Fail safety flag reference during ACTIVE | Fail closed to output fault, brake and release |

All numerical values above are fixtures, not measured robot calibration.
S2-U01/U02 still require target evidence.

### Correction acceptance results

Tested base HEAD: `4240895a455679ad0a3996057e03723cf2e9daea` on `main`,
with uncommitted Buddy 2 corrections. These are preliminary working-tree
results, not evidence of a clean tested commit. The metadata records Git
status, UTC start, options and SHA256 for all 42 Buddy 2 C/H/Python/recipe
files. Final host and ARM metadata hashes were checked against current source.

| Activity | Actual result | Evidence |
|---|---|---|
| Final default host: GCC 14.2.0, strict C99 and `-fanalyzer` | S2-U03–U120: 118 unique PASS groups; no warning/error diagnostics | [Raw output](logs/2026-10-15_postaudit/verify_postaudit20261015b_default.log) |
| Final optimized host: same, `-O2 -fshort-enums` | S2-U03–U120: 118 unique PASS groups; no warning/error diagnostics | [Raw output](logs/2026-10-15_postaudit/verify_postaudit20261015b_optimized.log) |
| Exact host source/revision metadata | 42 source hashes match final implementation | [Metadata](logs/2026-10-15_postaudit/verify_postaudit20261015b_metadata.json) |
| ARM GCC 10.3.1 object compilation using saved professor Make recipe, strict application warnings | 20 C files x SMP=0/1 x TEST=0/1 = 80 successful compiles; no warning/error diagnostics | [Raw compiler output](logs/2026-10-15_postaudit/verify_postauditarm20261015a_arm.log), [metadata](logs/2026-10-15_postaudit/verify_postauditarm20261015a_metadata.json) |
| Host-only guard without host mode | Expected compiler rejection PASS | [Raw output](logs/2026-10-15_postaudit/guard_host_required.log) |
| Host-only guard with physical target macro | Expected compiler rejection PASS | [Raw output](logs/2026-10-15_postaudit/guard_hardware_rejection.log) |
| Style subset: EOF, width, tabs, trailing whitespace, module filename prefix | PASS: 19 checked files; console result, no separate raw log | `python -B app_program/subsystem2/verify_motion.py --style` |
| Python syntax | AST parse PASS; console result, no separate raw log | `ast.parse` of verification script |
| Full integrated firmware link, on-target/SMP timing, physical movement | NOT RUN | Compatible SDK and integration/hardware gates remain unresolved |

Final host command from repository root:

```text
python -B app_program/subsystem2/verify_motion.py --label postaudit20261015b --analyze --output-dir testing/subsystem2/logs/2026-10-15_postaudit
```

ARM command (object compilation only; no Make invocation or link):

```text
python -B app_program/subsystem2/verify_motion.py --arm --strict --label postauditarm20261015a --output-dir testing/subsystem2/logs/2026-10-15_postaudit
```

The earlier `postaudit20261015a` host logs are preserved as intermediate
working-tree evidence: both profiles passed 95 pure and 23 adapter groups,
before the final startup/fault assertions and unique-ID checks were strengthened.
Their metadata describes that earlier source, not the final implementation.
Existing test groups and inherited demonstration tests were not removed;
S2-U01/U02 were not executed by either host profile. Fake kernel tests are
sequential and cannot establish real SMP ordering or physical emergency latency.
The Barr style subset and compiler checks are not a full Barr-C certification.
External kernel ABI names and test-only `main`/`setjmp`/`longjmp`, plus the
long scenario dispatcher in test_rtos.c, remain documented test-only exceptions.

Tracked-file hashes were compared with the clean starting HEAD snapshot:
all 933 tracked paths were checked; only the eight listed Buddy 2 paths
changed. Two new host-test C/H files and this run's evidence are Buddy 2-owned.
No teammate, shared startup/interface, Makefile, kernel, root AGENTS.md or
reference_docs file changed. Generated verification binaries/objects are
disposable and excluded from evidence; raw logs and metadata are retained.

### Post-merge evidence integrity manifest

SHA256 covers complete original bytes. Intermediate `a` and final `b` host
runs are distinct; no historical evidence was overwritten.

| Evidence | Bytes | SHA256 |
|---|---:|---|
| [guard_hardware_rejection.log](logs/2026-10-15_postaudit/guard_hardware_rejection.log) | 489 | `a040cd2790b4bf81e19185088741f1000f7ae152922a5fe48d2a5660be70390f` |
| [guard_host_required.log](logs/2026-10-15_postaudit/guard_host_required.log) | 447 | `bb7a2aef330cdc0b938063e977930c077ad789e4643aaf6f95bd2fd5461b24e6` |
| [verify_postaudit20261015a_default.log](logs/2026-10-15_postaudit/verify_postaudit20261015a_default.log) | 13779 | `59546a04c4e21dc16449159ae8e6338f41d176c7c736da2d61baa02a1577c664` |
| [verify_postaudit20261015a_metadata.json](logs/2026-10-15_postaudit/verify_postaudit20261015a_metadata.json) | 4659 | `c49c6873c16d3ed091408e76a1f1af23296cd416ef5cca2b497f088a05b66e49` |
| [verify_postaudit20261015a_optimized.log](logs/2026-10-15_postaudit/verify_postaudit20261015a_optimized.log) | 13867 | `9b5acb796ebcfb31e22b7dbd22ba9726017c426458e4e25eb355d2cc1b2ccbeb` |
| [verify_postaudit20261015b_default.log](logs/2026-10-15_postaudit/verify_postaudit20261015b_default.log) | 13779 | `59546a04c4e21dc16449159ae8e6338f41d176c7c736da2d61baa02a1577c664` |
| [verify_postaudit20261015b_metadata.json](logs/2026-10-15_postaudit/verify_postaudit20261015b_metadata.json) | 4729 | `5ce927f6879447c91ec557b11d93e98d8affb6f5eef82166d477375c24e5d03b` |
| [verify_postaudit20261015b_optimized.log](logs/2026-10-15_postaudit/verify_postaudit20261015b_optimized.log) | 13867 | `9b5acb796ebcfb31e22b7dbd22ba9726017c426458e4e25eb355d2cc1b2ccbeb` |
| [verify_postauditarm20261015a_arm.log](logs/2026-10-15_postaudit/verify_postauditarm20261015a_arm.log) | 403668 | `9066890d4d06c8ecce164fecdcacbacebc8e63400716161f0a860aa254adb8b6` |
| [verify_postauditarm20261015a_metadata.json](logs/2026-10-15_postaudit/verify_postauditarm20261015a_metadata.json) | 4731 | `d20576e7e03b77662d9c71796ef4c820f621063d25a646a529f7a3f8aa1540aa` |

## Final lifecycle and hygiene correction — test definitions

S2-U98–U120 retain their IDs and behaviours. Cleanup now checks object identity,
type, liveness and lock state; U99–U103 also retry after complete rollback and
reject repeated successful initialization. U108 asserts the retained safety bit;
U114/U115 assert task exit; U116 establishes fresh validity/standstill before
aging and checks unchanged publication and measurement timestamps. Encoder
failure cases assert the acquisition stage, and release requires live ownership.
The same host-only harness now compiles the actual subsystem2.c wrapper.

| ID | Definition / method | Expected result |
|---|---|---|
| S2-U121 | Fail demonstration semaphore creation, then retry | No live objects/start; retry succeeds; repeated init preserves IDs |
| S2-U122 | Fail demonstration pool creation, then retry | Owned semaphore released; retry succeeds; no duplicates |
| S2-U123 | Fail motion mutex creation inside subsystem init | Both demonstration objects rolled back; retry succeeds |
| S2-U124 | Fail motion wake creation inside subsystem init | Exact motion/demo resources released; retry succeeds |
| S2-U125 | Fail motion safety creation inside subsystem init | Exact motion/demo resources released; retry succeeds |
| S2-U126 | Inject motion spin initialization error inside subsystem init | Exact resources released; retry succeeds; synthetic defensive failure |
| S2-U127 | Fail motion task creation inside subsystem init | All five created objects released; retry succeeds |
| S2-U128 | Successful init/start then repeat | Repeated calls reject; IDs/live objects preserved; one producer and one motion start |
| S2-U129 | Start before initialization | E_OBJ; no producer, motion start or object creation |
| S2-U130 | Motion start fails after producer starts | Original error; published objects retained; repeated start cannot duplicate producer |
| S2-U131 | Producer helper fails before motion start | Original error; motion not started; repeated start rejected |
| S2-U132 | Demonstration rollback deletion fails | First init error retained; live ID retained; retry quarantined with E_OBJ |
| S2-U133 | Motion rollback deletion fails | First init error retained; live ID retained; retry quarantined with E_OBJ |
| S2-U134 | Probe wrong-type, duplicate and invalid fake deletions | Each rejected without changing remaining live objects; legitimate delete succeeds |

Runner checks additionally exercise the actual timeout handler, both host-only
compile guards and seven temporary production mutations. Mutation cases must
compile successfully and fail the designated test; a compiler failure is not
counted as detection. No mutation is applied to repository production source.
Executed results are recorded below. These remain non-actuating host tests.

Superseded combined technical/handoff guide preserved before consolidation: [original bytes in archive](logs/2026-10-15_hygiene_intermediate_evidence.zip) (entry `2026-10-15_superseded_motion_system.txt`), 39867 bytes, SHA256 `c8e0ac72dfed986d4abcb0b8f91b0fe55f3e75c9a8c3aeb232b1cbc4ab631351`. Relative links in this raw historical artifact describe its original location.

### Final correction acceptance — 2026-10-15

Base HEAD: `923f6ce56854ef9281584b43fd85e92888eb81cf`, branch `main`, plus
uncommitted Buddy 2 corrections. The earlier new files were already staged by
the user at entry; this pass did not alter the index. These are preliminary
working-tree results, not a clean commit-verified implementation.

| Date / executor | Test IDs / activity | Actual result | Evidence |
|---|---|---|---|
| 2026-10-15 / Codex host automation | S2-U03–U134, default GCC 14.2.0 strict C99, -fanalyzer | 132 unique PASS groups: 95 existing pure + 37 adapter/lifecycle; no warnings/errors | [Final default](logs/verify_hygiene20261015e_default.log) |
| 2026-10-15 / Codex host automation | Same IDs, -O2 -fshort-enums | 132 unique PASS groups; no warnings/errors | [Final optimized](logs/verify_hygiene20261015e_optimized.log) |
| 2026-10-15 / Codex runner checks | Timeout handler, physical-target/host-mode guards, seven temporary mutations | Expected timeout EXIT 124; guard rejections; all seven compiled mutations detected through eight designated FAIL results | [Check output](logs/verify_hygiene20261015e_checks.log) |
| 2026-10-15 / Codex ARM compiler | ARM GCC 10.3.1; 19 firmware C files x SMP=0/1 x TEST=0/1 | 76 successful object compiles; no warning/error diagnostics; no linking/execution | [Final ARM](logs/verify_hygienearm20261015d_arm.log) |
| 2026-10-15 / Codex static checks | Style subset, Python AST, test-ID registry, documentation file links, archive integrity | PASS: 19 style files; 134 unique definition IDs U01–U134; all current file links resolve; archive bytes/hashes match | Console checks; source/evidence manifests below |
| Not executed | S2-U01/U02, integrated Make link, real SMP/IRQ/timing, physical hardware | NOT VERIFIED by these runs | Compatible SDK, shared integration and separately authorised target/hardware procedures required |

Final commands from repository root:

```text
python -B testing/subsystem2/verify_motion.py --label hygiene20261015e --analyze
python -B testing/subsystem2/verify_motion.py --arm --strict --label hygienearm20261015d
python -B testing/subsystem2/verify_motion.py --style
```

[Host metadata](logs/verify_hygiene20261015e_metadata.json) and
[ARM metadata](logs/verify_hygienearm20261015d_metadata.json) each identify all
42 production/test/runner/recipe source files using repository-relative paths.
Every final source hash was compared with the actual working tree and matched.
Host suite/check commands use bounded subprocess execution. Compile failures
are not counted as mutation detection. The mutation list is specific regression
coverage, not exhaustive proof that every possible defect will be detected.

The first hygiene attempt (`hygiene20261015a`) failed before executing tests:
the sandbox temporary directory was inaccessible to GCC, and its cleanup also
reported access denied. No test PASS is attributed to that attempt. The runner
now uses the existing ignored testing/subsystem2/build location, private temporary
subdirectories and no shared configuration changes. Parent deletion was removed
during self-review to avoid races between concurrent host/ARM invocations.
The final execution left no executables, objects, dependencies, stack-usage,
firmware images, cache files or temporary subdirectories in Buddy 2 paths.

Intermediate runs a–d and ARM a–c, including the failed attempt, plus the
superseded guide are consolidated in
[one historical archive](logs/2026-10-15_hygiene_intermediate_evidence.zip).
Its MANIFEST.json records original names, lengths and SHA256 for 21 entries.
Every original byte was compared with the archived entry before removing the
untracked loose copies. No unique contents or committed/staged evidence were
deleted. Prior 2026-10-14/15 evidence, including the staged postaudit directory,
remains unchanged. Historical numerical results/commands remain historical.

Independent self-review: original creation errors are preserved; cleanup checks
positive owned IDs and retains IDs when deletion fails; pre-start complete
rollback allows retry; successful/repeated init and repeated starts cannot
duplicate published resources/tasks. Failed starts retain demonstration objects
because other buddies can already be using them. Only the actual wrapper's side
of app_start_task is tested; the shared helper masks some kernel start errors
and requires separate team review. No production motion-control algorithms or
original test assertions were removed/weakened. The producer implementation,
SAMPLE_MSG layout and demonstration semaphore/pool settings are preserved.

Barr-C review covers new lifecycle braces, boolean names, explicit error flow,
module visibility, source ordering and unchanged bounds/units. Supplied wrapper
signatures and existing demonstration statements retain their template style;
host-only ABI doubles, main/setjmp/longjmp and the scenario dispatcher retain
the documented test-only exceptions. Neither style subset nor warnings certify
complete Barr-C compliance or target timing/safety.

All 958 tracked paths at entry were hash-compared for ownership. Only Buddy 2
source/test/document paths changed or moved. Root AGENTS.md, reference_docs,
.gitignore, Makefiles, kernel/RTOS, shared startup/interfaces, other buddies and
integration/system test folders were not changed. HEAD and index hashes matched
entry. The user's staged snapshot remains older than the final working tree:
its staged source-folder test additions now have working-tree deletions, and
relocated files/final changes remain unstaged. Do not confuse that snapshot with
the reviewed final layout. No staging, commit, push, merge or branch operation
was performed.

Current organisation has one guide (docs/buddy2/MOTION_SYSTEM.md), one runner,
one host-only harness, and one test/evidence index here. Existing firmware tests
remain in the Make-discovered source folder without duplicate copies. The runner
and recipe live together in testing/subsystem2. The existing 'build' ignore rule
covers all runner-generated artifacts; optional scoped Python-cache exclusions
may be proposed separately if running Python without -B. No Make/ignore change
is needed for the verified layout. Unsupported runtime restart and incomplete
rollback recovery deliberately require reboot, not a new lifecycle manager.

### Final evidence integrity manifest

| Evidence | Bytes | SHA256 |
|---|---:|---|
| [verify_hygiene20261015e_default.log](logs/verify_hygiene20261015e_default.log) | 16044 | `120d78ab55dcea6f181da1c576c54823476d63f6ef48279386345bf6097e03c2` |
| [verify_hygiene20261015e_optimized.log](logs/verify_hygiene20261015e_optimized.log) | 16160 | `059f79a9b8808b7ea70b63e4bcfbc298aaae66ee4030b60b098f4e44052fef67` |
| [verify_hygiene20261015e_checks.log](logs/verify_hygiene20261015e_checks.log) | 17416 | `8d5565bc354bc50e8dda1421084ecd39fcc3c07d408b64ad7928662b8d0b6b41` |
| [verify_hygiene20261015e_metadata.json](logs/verify_hygiene20261015e_metadata.json) | 8120 | `bfb5515e6e6bfbea73be6739ffb432bdfe698c01373b91f3e0f249bff857449f` |
| [verify_hygienearm20261015d_arm.log](logs/verify_hygienearm20261015d_arm.log) | 378324 | `1457751957fef813aa5b1f8990e551a6bf688407947e5369e292f3b49debc1ce` |
| [verify_hygienearm20261015d_metadata.json](logs/verify_hygienearm20261015d_metadata.json) | 8381 | `bebcaa012be4fa927e73349bb4e8ff8dae6dfa2ea775db7867d88bcdbc7e5648` |
| [2026-10-15_hygiene_intermediate_evidence.zip](logs/2026-10-15_hygiene_intermediate_evidence.zip) | 76721 | `b6c7a7425926547d6e9ec77aee5033a02917aaa2716784a29f2e3dba57dd5541` |

### Final working-tree file inventory against HEAD

This inventory includes prior staged evidence carried into this pass. It is
not an instruction to stage/commit. Four host tools moved from app_program/
subsystem2 to testing/subsystem2: test_rtos.c/.h, verify_motion.py and the
byte-preserved m2_make_recipes.txt. Git rename classification is deferred until
the user updates the index; no source/test copy remains at its former location.

```text
M	app_program/subsystem2/README.md
M	app_program/subsystem2/drv_encoder.c
D	app_program/subsystem2/m2_make_recipes.txt
M	app_program/subsystem2/subsystem2.c
M	app_program/subsystem2/subsystem2.h
M	app_program/subsystem2/task_motion.c
M	app_program/subsystem2/task_motion.h
M	app_program/subsystem2/test_drive.c
D	app_program/subsystem2/verify_motion.py
M	docs/buddy2/MOTION_SYSTEM.md
M	docs/buddy2/MOTOR_DRIVER.md
M	testing/subsystem2/TEST_LOG.md
A	testing/subsystem2/logs/2026-10-15_postaudit/guard_hardware_rejection.log
A	testing/subsystem2/logs/2026-10-15_postaudit/guard_host_required.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015a_default.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015a_metadata.json
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015a_optimized.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015b_default.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015b_metadata.json
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015b_optimized.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postauditarm20261015a_arm.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postauditarm20261015a_metadata.json
```

Additional final files (untracked; not included in the diff above):

```text
testing/subsystem2/logs/2026-10-15_hygiene_intermediate_evidence.zip
testing/subsystem2/logs/verify_hygiene20261015e_checks.log
testing/subsystem2/logs/verify_hygiene20261015e_default.log
testing/subsystem2/logs/verify_hygiene20261015e_metadata.json
testing/subsystem2/logs/verify_hygiene20261015e_optimized.log
testing/subsystem2/logs/verify_hygienearm20261015d_arm.log
testing/subsystem2/logs/verify_hygienearm20261015d_metadata.json
testing/subsystem2/m2_make_recipes.txt
testing/subsystem2/test_rtos.c
testing/subsystem2/test_rtos.h
testing/subsystem2/verify_motion.py
```

### Approved Git preparation — 2026-10-15

Preparation runs on buddy2-Motor-and-encoder-control at base HEAD
923f6ce56854ef9281584b43fd85e92888eb81cf. The user authorised reconciling the
older index with the final working tree and creating one Buddy 2 commit.
Final host/ARM evidence above was rechecked against all 42 current source
files. These remain pre-commit executions; no post-commit rerun is claimed.
Git normalises CRLF to LF in text blobs. The sorted repository-path to SHA256
map of those 42 sources, with only CRLF-to-LF normalisation, has aggregate
SHA256 `cfc7e06f307e71e795c4d9239c6e1689fbf03ff1be769f41225a6f73cdb85636`. The digest is SHA256 of UTF-8 JSON, sorted keys,
compact comma/colon separators. Staged and committed blobs must reproduce
that digest; historical raw-file hashes remain unchanged. The resulting
commit hash is reported separately to avoid a self-referential commit record.

### Reconciled commit inventory

The approved single commit contains exactly these 32 changed entries.
Rename source/destination are shown together; obsolete source-folder host-test
additions are absent. This list was checked against the reconciled index.

```text
M	app_program/subsystem2/README.md
M	app_program/subsystem2/drv_encoder.c
M	app_program/subsystem2/subsystem2.c
M	app_program/subsystem2/subsystem2.h
M	app_program/subsystem2/task_motion.c
M	app_program/subsystem2/task_motion.h
M	app_program/subsystem2/test_drive.c
D	app_program/subsystem2/verify_motion.py
M	docs/buddy2/MOTION_SYSTEM.md
M	docs/buddy2/MOTOR_DRIVER.md
M	testing/subsystem2/TEST_LOG.md
A	testing/subsystem2/logs/2026-10-15_hygiene_intermediate_evidence.zip
A	testing/subsystem2/logs/2026-10-15_postaudit/guard_hardware_rejection.log
A	testing/subsystem2/logs/2026-10-15_postaudit/guard_host_required.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015a_default.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015a_metadata.json
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015a_optimized.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015b_default.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015b_metadata.json
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postaudit20261015b_optimized.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postauditarm20261015a_arm.log
A	testing/subsystem2/logs/2026-10-15_postaudit/verify_postauditarm20261015a_metadata.json
A	testing/subsystem2/logs/verify_hygiene20261015e_checks.log
A	testing/subsystem2/logs/verify_hygiene20261015e_default.log
A	testing/subsystem2/logs/verify_hygiene20261015e_metadata.json
A	testing/subsystem2/logs/verify_hygiene20261015e_optimized.log
A	testing/subsystem2/logs/verify_hygienearm20261015d_arm.log
A	testing/subsystem2/logs/verify_hygienearm20261015d_metadata.json
R100	app_program/subsystem2/m2_make_recipes.txt	testing/subsystem2/m2_make_recipes.txt
A	testing/subsystem2/test_rtos.c
A	testing/subsystem2/test_rtos.h
A	testing/subsystem2/verify_motion.py
```


### Signed encoder feedback - definitions before execution

| ID | Definition | Method | Expected result |
|---|---|---|---|
| S2-U135 | Full int64 paired count precision and valid first baseline without speed | actual motion_run with existing motor fakes | Exact INT64_MIN/MAX; first baseline counts valid, speed invalid |
| S2-U136 | Partial estimator failure cannot publish a mixed encoder pair | actual motion_run, invalid right conversion after left accepts | Fault invalidates counts; previous pair/time/generation retained |
| S2-U137 | Sub-period, STOP and fault publication never re-stamp counts | actual motion_run with motor fakes | No premature pair replacement; emergency invalidates retained history |
| S2-U138 | Missing/mismatched acquisition generations rejected | actual motion_run | Sensor fault before accepting first pair |
| S2-U139 | Count-only status freshness, unavailable/BUSY and clock failures | real task/API with kernel/encoder fakes | Boundary accepted; stale/future/clock error invalid; cached snapshot preserved |
| S2-U140 | Paired signed ISR feedback and quiescent release on emergency | real task/encoder adapter with existing fakes | +1/-1 pair with same timestamp/generation; fault invalidates; one release |

Existing S2-U116/U119 additionally assert count validity/history and live count
publication. All previous 132 host groups and stable IDs are retained. These
new cases are host-only; no physical GPIO is initialized by automatic tests.

### Signed encoder feedback results (2026-10-15, uncommitted)

Branch `codex/buddy2-motion-feedback`, based on main
`f3e77ffb5fc0d1ddeb7b963bed939fa926db89fc`. These are preliminary working-tree
results, not a new tested commit. No staging, commit, push, merge or hardware use.

Commands from root, using the same installed Python executable as prior runs:

```text
python -B testing/subsystem2/verify_motion.py --label feedback_reviewed --strict --analyze
python -B testing/subsystem2/verify_motion.py --label feedback_arm_reviewed --arm --strict
python -B testing/subsystem2/verify_motion.py --style
```

| Verification | Actual result | Evidence |
|---|---|---|
| Default host | 138/138 PASS: original 132 + 6 feedback cases | [log](logs/verify_feedback_reviewed_default.log) |
| O2/short-enum host | 138/138 PASS | [log](logs/verify_feedback_reviewed_optimized.log) |
| Timeout, hardware guards, focused mutations | PASS; 8 mutations detected, including int32 count truncation; expected timeout exit 124 | [checks](logs/verify_feedback_reviewed_checks.log) |
| Buddy 2 ARM objects | 76/76 PASS: 19 sources x SMP=0/1 x TEST=0/1 | [log](logs/verify_feedback_arm_reviewed_arm.log) |
| Existing integration consumers | 45/45 PASS per host profile, strict warnings/analyzer | [commands/results/source hashes](logs/verify_feedback_consumers.log) |
| Unchanged ARM controller and S4 consumer | 20/20 PASS: vehicle/adapter/dispatcher/app_main/imu_link x four profiles | [commands/results](logs/verify_feedback_arm_consumers.log) |
| Style subset | PASS, 19 existing checked files | Existing runner; no formal Barr-C certification |
| Full linking, target RTOS, physical calibration | NOT RUN | Required SDK 2.2.0 remains unavailable in the checked environment; no shared build/SDK patch |

No compiler warnings/analyzer findings in changed Buddy 2 code. The consumer
host build preserves the already documented S1 codec conversion warnings,
without modifying its source; only that provider object relaxes those two
warning classes. ARM strict checking retains the existing documented exclusions
for inherited/template/test objects. Real firmware linking was not attempted.

Both final Buddy 2 metadata files match all 42 current C/H/Python/recipe inputs:
[host metadata](logs/verify_feedback_reviewed_metadata.json),
[ARM metadata](logs/verify_feedback_arm_reviewed_metadata.json). Raw SHA-256 values
refer to compiler input bytes; Git CRLF/LF normalization must be accounted for
at commit. The sorted path-to-Git-blob-ID mapping of these 42 inputs has SHA-256
`b11e4f6dab1d585d752f4d82313c29f8a43cf52ce44d9b677d5d4a9a29f4a9d1`. Digest input is UTF-8 JSON, sorted keys and compact comma/colon
separators; Git IDs use `git hash-object --path=<repository path> <file>`.

ARM `sizeof(motion_run_status_t)` increased from 104 to 136 bytes (+32). The
existing owner/run and bus status copies each grow by 32 bytes, as do consumer
stack instances; no task, lock, allocation, hardware operation or PID deadline
was added. Full stack headroom/ISR latency still requires target verification.

Independent review: counts are copied from the actual accepted capture, never
from distance. Copy occurs only after both estimators accept; faults invalidate
without replacing history. First-baseline counts do not authorize speed/motion.
Malformed zero/mixed generations now fault before acceptance. ISR feed/snapshot
remain under the same interrupt-safe spinlock; the complete published struct
remains under the existing inheritance mutex. Fakes assert lock balance and
quiescent release, not actual multicore memory ordering or timing. All original
tests remain; U116/U119 and existing fault-path validity assertions were
strengthened. The shared dispatcher,
controller, other buddies, Makefiles, kernel, main and Git index are unchanged.

Shared agreement still required: signed telemetry range/encoding, count validity,
sample timestamps and reboot/session handling. The controller must explicitly
consume the new fields; neither its telemetry nor Buddy 1's structures changed.
Physical wiring, polarity, wheel/turn/PID/stop calibration and end-to-end stop
latency remain outstanding. No live reset manager or recovery was introduced.
Recommendation: PASS WITH CONDITIONS for review/commit of this source-compatible
interface extension; all affected firmware objects must be rebuilt together.

Intermediate successful runs are preserved byte-for-byte in
[development archive](logs/feedback_development_evidence.zip), with its embedded
SHA-256 manifest. No previous committed test history was removed. Compiler
objects, executables, size probes and .su files were temporary under ignored
Buddy 2 build scratch and are removed. No new runner or folder hierarchy.

Changed production/test/document files:

- `app_program/subsystem2/motion_api.h`
- `app_program/subsystem2/motion_run.c`
- `app_program/subsystem2/motion_run.h`
- `app_program/subsystem2/task_motion.c`
- `docs/buddy2/MOTION_SYSTEM.md`
- `testing/subsystem2/TEST_LOG.md`
- `testing/subsystem2/test_rtos.c`
- `testing/subsystem2/verify_motion.py`

| Evidence | SHA-256 (working bytes) |
|---|---|
| `feedback_development_evidence.zip` | `f9c70483d625b51449ebe4a299fa63391f4223ca79e46189ff4eb7f1a7f2161c` |
| `verify_feedback_arm_consumers.log` | `8e2cb6dd232c1a6985b35153257e3ef14e23aa51be38e0827884a0b7b405cec1` |
| `verify_feedback_arm_reviewed_arm.log` | `1925b69877f6f869f8748ad675ed7ed78ecacdc81814bfbf4c0f9d3213cf8ff7` |
| `verify_feedback_arm_reviewed_metadata.json` | `15b54b32a2a63a5402a022fddf732a6f18385f87af826cf041f8fd9abba6d35c` |
| `verify_feedback_consumers.log` | `29abc35995d291965386d306a06f4226f85a0e5c74ed3824e5ced454574fbe50` |
| `verify_feedback_reviewed_checks.log` | `d1b548c059128ef1ac617c1af6a7139452eb63bd7d0581adeb47d37488fe57d1` |
| `verify_feedback_reviewed_default.log` | `c010bf08b1d3c2e2aac9ed231652a40f77d5d6526054a3ad98c90e5bae10a0ff` |
| `verify_feedback_reviewed_metadata.json` | `d214685a35ac563e03788a45bd276155968681079b3f4d3f79e148b349b11c77` |
| `verify_feedback_reviewed_optimized.log` | `5f332f63f23ff0e4b39ed0338c366df847f3c1f81aa1d657011d497d60eafc9c` |

## Minimum-bit before-change baseline - 2026-10-21

Tested production revision: `d4c91f271c9801802e3994967c96e9ec8fcba38c`,
branch `buddy2-Motor-and-encoder-control`, initially clean and identical to
local main. Only new measurement tooling, documentation and evidence were
added during this run; no production source changed. No commit/push/flash.

| Existing checks | Actual outcome | Evidence |
|---|---|---|
| S2-U03..U140 default/analyser | 138 groups PASS | [default](logs/verify_minbits_before_20261021_default.log) |
| S2-U03..U140 O2/short-enums/analyser | 138 groups PASS | [optimized](logs/verify_minbits_before_20261021_optimized.log) |
| Runner timeout, host guards, eight mutations | PASS; expected timeout/rejections not product failures | [checks](logs/verify_minbits_before_20261021_checks.log) |
| ARM strict objects | 19 x SMP0/1 x TEST0/1 = 76 PASS | [ARM retry](logs/verify_minbits_before_20261021_arm_retry_arm.log) |
| ARM structure inventory | 35 structures / 299 member declarations measured | [inventory](logs/minbits_layout_before_20261021_retry/inventory.md), [JSON](logs/minbits_layout_before_20261021_retry/layout.json) |
| Isolated candidate layouts | Compile/size evidence only; no behaviour claimed | [candidate sizes](logs/minbits_layout_before_20261021_retry/candidate_sizes.json) |
| Six full Make profiles | ELF/UF2 builds PASS; example credential fixture only | [results](logs/minbits_firmware_before_20261021/results.json) |

Commands from repository root:

```text
python testing/subsystem2/verify_motion.py --analyze --label minbits_before_20261021
python testing/subsystem2/verify_motion.py --arm --strict --label minbits_before_20261021_arm
python testing/subsystem2/verify_motion.py --arm --strict --label minbits_before_20261021_arm_retry
python testing/subsystem2/measure_representation.py --label minbits_layout_before_20261021
python testing/subsystem2/measure_representation.py --label minbits_layout_before_20261021_retry
```

The first ARM run could not launch the compiler inside the restricted process
environment (Windows 623). The approved outside-restriction retry passed with
existing ARM GCC 10.3.1. Host GCC 14.2.0. Initial layout probe failed because
its generated includes omitted SAMPLE_MSG; corrected probe passed. Failed logs
are preserved; neither attempt is counted as a successful compile.

Full builds used existing MSYS2 bash/Make and repository-local SDK, not WSL
(only docker-desktop was listed). Reproducible shell and orchestration scripts
are copied into the firmware evidence directory. Invocation from build_make:

```text
export PICO_SDK_PATH=../build/deps/pico-sdk
make [profile options] "INCPATH=-I../include -I../config -I../kernel/knlinc -I../build/profiles" -j8
```

All use SMP=1 CONSOLE=usb_cdc WIFI=cyw43 CRASH_REPORT=0. Profiles: default;
TEST=1; S4_IMU=1; S4_IMU=1 TEST=1; NET=lwip MQTT=1 S4_IMU=1;
NET=lwip MQTT=1 S4_IMU=1 TEST=1. Example fixture hash matches the published
config/wifi_secrets.example.h; no live credentials/network operation.

| Fresh profile | text | data | bss bytes |
|---|---:|---:|---:|
| default | 345808 | 92 | 45156 |
| TEST | 434464 | 1160 | 54004 |
| IMU | 359972 | 1168 | 45168 |
| IMU TEST | 437136 | 1168 | 54012 |
| MQTT IMU | 419380 | 1176 | 102824 |
| MQTT IMU TEST | 492504 | 1176 | 111668 |

These fresh section sizes differ from existing October 16 images captured in
layout.json. Cause is not established by this run. Future savings must compare
matched fresh builds, not mix those artifacts. Static sections do not measure
runtime heap/stack peaks, control/IRQ timing or physical behaviour.

Measured before: motion_bus_request_t 256 B, motion_bus_t 400 B,
motion_run_status_t 136 B. Candidate tagged union request 216 B, retaining all
numerical widths. Selected bool packing has some savings and some zero-saving
cases. Candidates are declarations only, not modified firmware or completed
minimum-bit compliance. Proposed shared changes await concrete review in
[data representation review](../../docs/buddy2/DATA_REPRESENTATION.md).

### Isolated proposed-patch verification

The eight-file proposal is applied ONLY in ignored
testing/subsystem2/build/minbits_candidate_review. Actual app_program remains
unchanged. The draft includes selected owner bool fields, an internal tagged
request union and scalar flags in shared vehicle.h. No numeric field shrinks.

| Candidate checks | Actual result |
|---|---|
| Existing Buddy 2 suites | 138 groups/profile and eight mutations PASS |
| Existing integration suites | 79 unique INT IDs/profile including TEST startup; eight mutations/profile PASS; existing S5 suites PASS |
| Strict ARM | 76 Buddy 2 and 44 shared/provider objects PASS |
| Final formatting | whitespace-only equivalence checked; 19-file style subset PASS |

Raw logs/metadata are retained under
logs/minbits_layout_before_20261021_retry/candidate_subsystem2_evidence and
candidate_integration_evidence. Initial ARM candidate attempts lacked an
unmodified device header in the isolated copy; retries passed after copying
tracked headers. Failures are retained, not counted as product regressions.

Commands used the existing verify_motion.py and verify_integration.py in that
isolated tree, labels minbits_candidate_review, minbits_candidate_review_arm
and minbits_candidate_review_arm_retry. The review diff is
logs/minbits_layout_before_20261021_retry/proposed_first_patch.diff.txt.
Formatting hashes and exact measured nested sizes are beside it. Existing
behavioural results bind the pre-formatting source; whitespace-only final
formatting is recorded explicitly. New representation-specific tests and full
candidate firmware links remain required after approval to apply the patch.

## Merged minimum-bit increment - 2026-10-28

Definitions before implementation. Baseline HEAD
`93740736dd1c3407eaf7f496089a096cee70c140` on
`buddy2-Motor-and-encoder-control`, identical to local main. October 21
uncommitted documentation/evidence is preserved as history, not relabelled as
this revision's verification. No teammate production implementation is in scope.

| ID | Method | Expected result |
|---|---|---|
| S2-U141 | Pure host and synthetic TEST worker | All combinations of compact motor/decoder/loop scalar flags survive assignment/copy/clear without changing adjacent data |
| S2-U142 | Actual one-slot bus with synthetic requests | CONFIG, COMMAND, ACK and payload-free ENABLE copy their selected variant; changing sender storage cannot change the copied request |
| S2-U143 | Actual one-slot bus with invalid tags | Invalid positive/negative request kinds reject without changing pending/inflight/stop state or retained payload |

Planned implementation: compact only scalar bool members of drv_motor_t,
motion_quad_t and wheel_loop_t, and replace mutually exclusive internal request
storage with a named tagged union. All numerical widths, public motion API
signatures, encoder locking and emergency signalling remain. Review/test each
variant access rather than applying the October 21 draft wholesale.
Shared vehicle.h and teammate code will not be edited in this increment.

### Implemented increment and actual results

Tested working tree: HEAD above plus seven modified S2 source/header files,
the host runner's expected-ID update and the representation probe. No peer or
shared production edits. The current data-representation guide records the
measured layouts and remaining range/interface decisions.

| Verification | Actual outcome |
|---|---|
| Motion baseline / after, default and optimized | 138 / 141 groups per profile PASS; eight mutations and timeout guards PASS |
| Integration before / after | 83 unique INT groups per profile including separate TEST startup PASS; eight mutations per profile and S5 suites PASS |
| Strict Buddy 2 ARM after | 19 objects x SMP0/1 x TEST0/1 = 76 PASS |
| Integration/provider ARM after | 11 objects x SMP0/1 x TEST0/1 = 44 PASS |
| S1 and S3 maintained host runners, before / after | PASS |
| Layout probe | Before 35 types/299 declarations; after 37/301; target sizes recorded |
| Full Make profiles before / after | All six links PASS; no compiler warnings in these full-build logs |
| Preservation | 78 peer and 631 shared/platform files byte-identical to pre-edit capture |
| Style / whitespace | 19-file S2 style subset PASS; git diff --check PASS |

Commands from repository root (existing tools only):

```text
python testing/subsystem2/verify_motion.py --analyze --label minbits_20261028_before
python testing/subsystem2/verify_motion.py --analyze --label minbits_20261028_after
python testing/subsystem2/verify_motion.py --arm --strict --label minbits_20261028_before_arm
python testing/subsystem2/verify_motion.py --arm --strict --label minbits_20261028_after_arm
python testing/integration/verify_integration.py --label minbits_20261028_before
python testing/integration/verify_integration.py --label minbits_20261028_after
python testing/integration/verify_integration.py --arm --label minbits_20261028_after_arm
python testing/subsystem1/host/verify_windows.py --log testing/subsystem2/logs/minbits_20261028_s1_after.json
python testing/subsystem3/verify_production_timing.py --gcc C:/msys64/ucrt64/bin/gcc.exe --log testing/subsystem2/logs/minbits_20261028_s3_after.json
python testing/subsystem2/measure_representation.py --label minbits_20261028_layout_before
python testing/subsystem2/measure_representation.py --label minbits_20261028_layout_after
python testing/subsystem2/build/minbits_make_20261028_before.py
python testing/subsystem2/build/minbits_make_20261028_after.py
python testing/subsystem2/verify_motion.py --style
git diff --check
```

S1/S3 baseline commands used the corresponding `_before.json` log names.
Saved full-build driver copies document the ignored helper scripts. Make runs
from build_make with -j8 and the explicit example-fixture include path.
All six use SMP=1 CONSOLE=usb_cdc WIFI=cyw43 CRASH_REPORT=0:

| Additional options | text before / after | data before / after | BSS before / after |
|---|---:|---:|---:|
| Defaults | 354360 / 354348 | 92 / 92 | 45220 / 45148 |
| TEST=1 | 452924 / 453920 | 1160 / 1160 | 54100 / 54028 |
| S4_IMU=1 | 369044 / 369032 | 1168 / 1168 | 45232 / 45160 |
| S4_IMU=1 TEST=1 | 455684 / 456680 | 1168 / 1168 | 54108 / 54036 |
| NET=lwip MQTT=1 S4_IMU=1 | 426948 / 426936 | 1176 / 1176 | 102888 / 102816 |
| NET=lwip MQTT=1 S4_IMU=1 TEST=1 | 511060 / 512056 | 1176 / 1176 | 111764 / 111692 |

Raw firmware results, section sizes, image hashes, commands and input manifests
are in logs/minbits_firmware_20261028_before and `_after`. Layout evidence is
in logs/minbits_20261028_layout_before and `_after`. Other logs use the command
labels above. logs/minbits_20261028_preservation.json records the byte audit.

Limits: existing SDK 2.2.0 and ARM GCC 10.3.1; compare this matched pair only.
These are host checks, ARM compilation and links, not execution of TEST on the
Pico. Runtime memory peaks, IRQ/control/emergency timing, electrical operation
and motion accuracy remain unmeasured. Numeric narrowing, public status/enums,
fault translation and shared vehicle flags remain open; no complete minimum-bit
compliance claim. No flashing, network access, installation, staging or commit.


## Real GP20 hard-fault correction - 2026-10-28

See the appended integration record "GP20 physical attempt: core-1 hard fault,
motor driver correction" for the actual COM7 capture and PC/disassembly
evidence. Fixed the S2 motor-brake GPIO_CTRL compound-argument access using
a precomputed pin_b, preserving the supplied macro. Full integrated IMU build
and 76 strict ARM object compilations passed on HEAD 0511f67 plus this fix.
Post-fix hardware movement remains NOT RUN; no physical PASS is claimed.

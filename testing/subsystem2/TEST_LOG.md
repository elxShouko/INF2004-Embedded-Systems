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
- [Foundation narrative](../../app_program/subsystem2/MOTION_FOUNDATION_VERIFICATION.md)
  and [motor narrative](../../app_program/subsystem2/MOTOR_DRIVER.md): preserved
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

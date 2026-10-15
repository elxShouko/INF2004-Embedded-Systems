# Buddy 2 — Motion Control

Buddy 2 owns both DC motors, encoders, wheel-speed/distance estimation, PID,
continuous differential speeds, distance/turn manoeuvres and motion results.
The Vehicle Controller owns mission decisions and command arbitration.

This is the normal project repository on `buddy2-Motor-and-encoder-control`.
All production and automated-test C/H files stay flat here for the professor's
Make source discovery. `subsystem2.c/.h` retain demonstration communication and
add motion task lifecycle hooks. Startup creates RTOS objects but does not
configure motor outputs or enable movement.

- `motion_config`, `motion_types`, `motion_quad`, `motion_estimator`,
  `motion_pid`: hardware-independent foundation.
- `drv_motor`, `drv_rp2040`, `drv_encoder`, `enc_capture`: output/acquisition.
- `wheel_loop`, `move_plan`, `motion_run`, `motion_bus`, `task_motion`: control,
  planning, lifecycle, copied intake and single-owner RTOS execution.
- `motion_api.h`: provisional controller interface; configuration and explicit
  enable are separate. Other subsystems must not use the motor backend.
- `subsystem2_test`, `motion_foundation_test`, `test_motor`, `test_drive`: tests.
- `verify_motion.py` and `m2_make_recipes.txt`: keep together; the script uses
  recorded Make compiler flags and locally installed compiler paths.

Technical details: [system](../../docs/buddy2/MOTION_SYSTEM.md),
[motor driver](../../docs/buddy2/MOTOR_DRIVER.md),
[foundation](../../docs/buddy2/MOTION_FOUNDATION_VERIFICATION.md).
Formal records and genuine archived evidence: [test log](../../testing/subsystem2/TEST_LOG.md).

From repository root, with Python and the script's compiler paths available:

```sh
python -B app_program/subsystem2/verify_motion.py --style
python -B app_program/subsystem2/verify_motion.py --label UNIQUE_RUN --analyze
```

The second command runs strict C99 host tests in default and O2/fshort-enums
profiles. It writes logs and generated executables here; archive genuine logs
under `testing/subsystem2/logs/` and exclude generated outputs from Git. Labels
must be unique to preserve evidence. Host tests use fake outputs, not GPIO.

Integrated firmware, after provisioning the documented SDK/toolchain:

```sh
cd build_make
make -j8
make TEST=1 -j8
```

The SDK 1.5.1 versus required 2.2.0 mismatch remains documented. ARM object
compilation does not establish full linking or target execution. Encoder GPIO
dispatcher hooks, controller contracts, pin/PWM ownership, task resources,
physical calibration and emergency-stop timing require team agreement and
target verification. No powered test is authorised by this implementation.

The producer, SAMPLE_MSG, sem_slots and mpf_samples remain solely because the
currently started Buddy 3 consumer and Buddy 4 monitor still depend on them.
Removing that chain requires coordinated retirement; do not disable it alone.
See [current handoff](../../docs/buddy2/MOTION_SYSTEM.md#normal-repository-handoff--2026-10-15).
Root AGENTS.md and reference_docs are approved to remain. Historical integrated
ARM/Make logs are archived under testing/subsystem2/logs; the final build_make
diff against local main is empty. See the [relocation record](../../testing/subsystem2/logs/2026-10-15_final_verification.md#integrated-arm-log-relocation--2026-10-15).
Full linking and shared integration still require review before release.

# Buddy 2 — Motion Control

Buddy 2 owns both DC motors, encoders, speed/distance estimation, PID,
continuous differential speeds and distance/turn execution. The Vehicle
Controller owns mission decisions and command arbitration.

Production C/H remain flat here for professor Make discovery. Existing dual
host/TEST=1 test sources remain here for that same discovery; they are not copied.
Host-only test_rtos.c/.h, verify_motion.py and m2_make_recipes.txt live together
in testing/subsystem2. Startup creates objects/tasks but never enables motors.
The demonstration producer and its communication dependencies are preserved.

- [Authoritative technical guide](../../docs/buddy2/MOTION_SYSTEM.md)
- [Test definitions, commands, results and evidence](../../testing/subsystem2/TEST_LOG.md)

From repository root:

```text
python -B testing/subsystem2/verify_motion.py --style
python -B testing/subsystem2/verify_motion.py --analyze --label UNIQUE_RUN
```

Generated artifacts use an automatically removed, ignored testing build
location. Evidence defaults to testing/subsystem2/logs; use a unique label.
Host tests and ARM compilation are not hardware validation. Full linking,
shared dispatcher/controller integration and physical calibration remain gates.

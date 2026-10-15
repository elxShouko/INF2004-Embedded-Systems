# Buddy 4 IMU tests

Historical suite: current firmware tests now live in
[`testing/buddy4`](../buddy4/README.md). The description below records the
earlier archived-source testing phase, not the current firmware state.

Owner: Eddy Teo Hon Kai (2501284), CSC12.

These C host tests exercise the existing IMU implementation preserved at
`82f2745754ef0c44df0579e3ee4c4f4709e75f47`, now also retained on local branch
`codex/buddy4-imu-archive-82f2745`. They do **not** test the monitor demonstration
currently in `app_program/subsystem4`, and do not establish integrated firmware
readiness. The source revision is fixed explicitly in the runner.

Run from the repository root with an installed host C compiler, Bash, Git, tar,
and shasum:

```sh
bash testing/subsystem4/run_host_tests.sh normal
bash testing/subsystem4/run_host_tests.sh sanitize
bash testing/subsystem4/run_host_tests.sh replay
```

`normal` runs 113 checks. `sanitize` repeats them under AddressSanitizer and
UndefinedBehaviorSanitizer. `replay` generates a synthetic RAW trace and feeds
it through the offline replay program; it is a demonstration, not a hardware
measurement or an assertion-based test. No downloads are performed. A shallow
clone lacking the pinned commit must obtain that history separately; the runner
fails instead of silently testing a different revision.

Every invocation saves a unique directory under `logs/` containing compiler
details, exact command, source revision, current harness HEAD, dirty status,
SHA-256 hashes of the test sources, and console results. Compiler/test failures
return nonzero. Temporary extracted sources and executables stay in the printed
temporary build directory for debugging. Update `TEST_LOG.md` with the run path.

`host/` contains the register simulator, scenario generator, C assertions,
driver/math/calibration/processing/hump/event cases, fault scenarios, adversarial
inputs, and six sequential fake-kernel checks. The latter require a full run:
they share wrapper lifecycle state and do not emulate scheduling or SMP memory
ordering. Do not treat their PASS results as evidence that the wrapper is safe
on the template's two cores.

All synthetic thresholds, noise levels, wheelbase values, speeds, and error
tolerances are test fixtures or archived development defaults. They are not
calibrated vehicle parameters. See `REFERENCE_REVIEW.md` for requirement
traceability, conflicts, and the implementation work still needed.

The existing S4-U01, S4-U02 and INT-02 monitor examples retain their meanings.
New stable IDs S4-U003 through S4-U115 identify the host cases; the complete
registry is `CASES.md`. Cases are registered before execution and never receive
IDs dynamically at runtime. A filtered run with no matches returns an error.

## Migration into the integrated template

The application foundation remains GNU Make / micro T-Kernel. Production IMU
code must ultimately live directly under `app_program/subsystem4/`, with drivers
named `drv_<name>.c/.h`. The host tests can then be retargeted to those files.
Do not copy the old standalone CMake integration into the integrated firmware.

Before retargeting, split RTOS creation from task start, correct shared-state
synchronisation, validate every startup/fault path, agree Mission Controller
events and telemetry units, and add the required SDK I2C/math dependencies.
The preserved monitor communication objects remain needed by the S3 consumer.
These are integration changes beyond this test-folder update.

## C coding standard scope

The migrated suite retains its established test cases so results can be compared
with the archived version. New assertion helpers evaluate operands once and use
fixed-width integers, explicit Boolean flags, and ordinary functions; macros
only capture the call-site location. The fake callback type has an explicit
prototype, and float-to-double conversions are explicit for strict Clang builds.

Full Barr-C:2018 compliance is **not claimed** for the imported legacy suite.
Known deviations include test/helper names without a module prefix, some long
functions, plain floating types, and compact struct declarations. The archived
fake kernel's `setjmp`/`longjmp` escape was removed: the pinned task's final
statement calls `tk_ext_tsk`, so the double can return normally. This double
does not model real task termination. The source implementation also
needs its own full standard review before adoption. Passing compiler warnings
or these tests cannot establish coding-standard conformance.

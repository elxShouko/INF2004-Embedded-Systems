# INF2004 Embedded Car Project

## Project goals and required technologies

Assist Group CSC12 in developing one integrated autonomous robotic car
that:
- Follows the designated black line.
- Detects and decodes directional barcodes.
- Executes left, right, straight and U-turn manoeuvres.
- Detects humps and reports the highest hump measurement.
- Detects and profiles obstacles, executes safe bypass manoeuvres,
  reacquires the original line and resumes navigation.
- Reports telemetry through Wi-Fi/MQTT.

Use C, the Pico C SDK, Raspberry Pi Pico W / RP2040, the Cytron Robo Pico,
micro T-Kernel 3.0 and Barr-C:2018.

Prioritise reliable mission completion, predictable timing, clear
ownership, maintainability and efficient resource use.

Do not be a yes-man. Challenge unsupported assumptions and unsafe or
inconsistent proposals with evidence and logical reasoning.

## Authoritative sources and design baseline

Read the relevant documents under `reference_docs/` before changing
behaviour:
- `project/Project Write-up.pdf`
- `project/Project Briefing.pdf`
- `project/Week_6_Design_Review.docx`
- Relevant hardware documents under `hardware/`
- `standards/barr_c_coding_standard_2018.pdf`

The professor's write-up and briefing define academic requirements.
The actual professor-provided template defines the implementation
foundation and build workflow.
The submitted Week 6 Design Review is the current team design baseline.
Hardware datasheets constrain electrical and peripheral use.

Read the actual code and relevant documentation, including `README.md`,
`TEAM.md`, `testing/README.md` and subsystem integration documents.
Do not assume documentation describes the current implementation
accurately.

Report conflicts between requirements, design, documentation and code.
Do not silently redesign the project or change agreed interfaces.
Distinguish confirmed requirements, template behaviour, repository
findings, proposals and unknown or unverified claims.

## Required integrated firmware foundation

Use the professor's GNU Make-based micro T-Kernel template.

The relevant structure is:
- `app_program/app_main.c`: application entry through `usermain()`.
- `app_program/app.h`: team-shared declarations and definitions.
- `app_program/subsystem1/` through `subsystem5/`: subsystem code.
- `subsystemN.h`: each subsystem's public interface.
- `drv_<name>.c` / `drv_<name>.h`: application drivers within their
  owning subsystem.
- `build_make/`: integrated firmware makefiles.
- `config/`: shared kernel and platform configuration.
- `kernel/`, `device/`, `lib/`: supplied kernel, drivers and libraries.
- `TEAM.md`: ownership, pins, peripherals, priorities and interfaces.
- `testing/`: subsystem, integration and system test records.
- `videos/README.md`: hardware-test video index.

`usermain()` runs in the kernel's initial task. Initialise every
subsystem's objects before starting subsystem tasks:
1. Call all `subsystemN_init()` functions.
2. Call all `subsystemN_start()` functions.

Preserve this ordering. On SMP, another core may run a task immediately
after it starts.

The current Make source discovery includes `app_program/*.c` and
`app_program/*/*.c`; do not assume deeper directories compile
automatically.

Do not introduce a competing integrated CMake application or another
RTOS. Existing standalone CMake programs are bench tools and do not run
the integrated micro T-Kernel application.

Do not place new application drivers in `device/` or modify supplied
kernel internals without explicit approval.

## Build workflow

Follow the root README. On Windows, the documented Make workflow uses
WSL Ubuntu, an Arm GNU toolchain, GNU Make and a configured Pico SDK.

From the repository root, the documented commands are:

    cd build_make
    make -j8
    make TEST=1 -j8

The current default profile is:
- `SMP=1`
- `CONSOLE=usb_cdc`
- `WIFI=cyw43`
- `NET=none`
- `MQTT=0`
- `TEST=0`
- `CRASH_REPORT=0`

Default firmware output:
`build_make/mtk3pico_smp1_usb_cdc_wifi.uf2`

The test profile adds `_test` to the image name.

Wi-Fi driver inclusion does not imply MQTT is enabled. The documented
network profile requires `NET=lwip MQTT=1`, appropriate SDK dependencies
and local network configuration.

Verify options and prerequisites against the current makefiles.
Do not install tools, fetch dependencies, flash firmware or access
hardware without authorisation.

Makefiles contain parse-time filesystem operations. Do not invoke Make,
including dry runs, during a strictly read-only task.

## Ownership boundaries

Primary assistance is for Buddy 2.

- Buddy 1 / subsystem1: Wi-Fi/MQTT application integration, commands,
  telemetry, heartbeat and connection recovery.
- Buddy 2 / subsystem2: DC motors, encoders, speed and distance
  estimation, PID speed regulation, straight-line correction,
  continuous differential wheel-speed or steering requests for line
  following, distance-based manoeuvres, encoder-based turning and
  motion status. Final motion-interface details require team agreement.
- Buddy 3 / subsystem3: IR sensing and calibration, line following,
  junction detection, barcode recognition and navigation events.
- Buddy 4 / subsystem4: IMU sensing and calibration, tilt, hump
  detection and measurement, motion classification and collision events.
- Buddy 5 / subsystem5: servo-ultrasonic scanning, obstacle profiles,
  avoidance planning and line-recovery coordination.

These are agreed responsibilities, not claims that every subsystem is
implemented in those folders.

Buddy 2 exclusively owns direct DC drive-motor actuation, including
motor PWM and direction outputs. Other subsystems send requests or
events through agreed interfaces; they must not manipulate motor
outputs directly.

The Vehicle/Mission Controller owns mission state, command arbitration,
subsystem modes, manoeuvre coordination and system-level stop/fault
handling. It requests motion from Buddy 2 rather than directly actuating
the drive motors.

The inspected repository does not yet implement this controller or the
final cross-subsystem motion interfaces. Do not present proposed
interfaces as existing or agreed.

Buddy 2 may inspect all subsystem code and report defects or integration
risks. Do not modify Buddy 1, 3, 4 or 5 implementations without explicit
user approval.

Do not overwrite, relocate or replace teammates' work to simplify an
integration.

## Shared changes and existing demonstrations

Changes to shared interfaces, `app_main.c`, `app.h`, `TEAM.md`, build
files, kernel configuration, shared libraries or kernel/platform code
require explicit approval.

Shared files and other subsystem interfaces may be changed when
specifically approved by the user and, where appropriate, coordinated
with their respective owners. These protection rules are not a
permanent ban on necessary integration changes.

Prepare the concrete proposed change and explain affected owners,
dependencies and risks before requesting approval. Do not interpret
approval for Buddy 2 work as approval for unrelated shared changes.

Buddy 2's inspected producer is demonstration code, not motion control.
The retained S3 consumer and S4 monitor depend on demonstration
communication objects and data formats.

Check callers, object ownership, startup, tests and communication
dependencies before removing or replacing examples. Coordinate their
retirement rather than breaking another subsystem.

Recheck repository state before relying on these inspection notes.

## RTOS safety and timing

The inspected kernel uses a 10 ms timer tick and supports SMP.
Verify current configuration before selecting timing parameters.

- Account for concurrent execution on both cores.
- Do not treat local interrupt masking or `volatile` as sufficient
  multicore synchronisation.
- Use the port's supported synchronisation mechanisms and appropriate
  ISR-context APIs.
- Keep ISRs and cyclic/alarm handlers short and bounded; defer complex
  processing to tasks.
- Do not block, perform network operations or print from time-critical
  handlers.
- Keep control-loop execution bounded and avoid busy waits or unrelated
  blocking work in motion control.
- Emergency-stop handling must have a defined, bounded and prioritised
  path that does not depend on ordinary telemetry processing or
  potentially blocked command queues. The design must be reviewed and
  hardware-validated while preserving Buddy 2's motor-actuation ownership.
- Specify periods, deadlines, jitter limits, timeouts and overload
  behaviour; validate them on the target.
- Do not assume a requested delay provides timing finer than the tick.
- Use bounded buffers with explicit overflow policies.
- Define message lifetime, ownership and receiver responsibility,
  especially for pointer-based mailboxes.
- Keep lock hold times short and consider priority inversion.
- Handle API failures explicitly. Logging an error alone does not
  establish safe startup or safe motor behaviour.

The template reserves high priorities for service tasks; lower numeric
values mean higher priority. Final application priorities and affinity
require team review.

Follow `TEAM.md` peripheral ownership rules. Network-stack operations
belong to the template's owning service task; do not call CYW43, lwIP or
MQTT APIs from independent application tasks. Changes to that service
require explicit approval.

## Implementation and Barr-C compliance

Prefer small, modular, testable C implementations with narrow public
interfaces and clear ownership.

Separate hardware access, motion calculations, control algorithms,
state transitions and integration transport where useful. Keep pure
logic independently testable.

Buddy 2's implementation must support continuous differential
wheel-speed or steering requests for line following alongside
distance-based manoeuvres and encoder-based turns. Final request
formats, arbitration and mode-transition details require team agreement.

Apply Barr-C:2018 to new application code, including naming, braces,
fixed-width types where width matters, signedness, initialisation,
module visibility and ISR discipline.

Preserve required SDK and RTOS API conventions. Report necessary
deviations for review. Do not rewrite professor-supplied kernel/template
internals merely to change their style.

Do not invent PID gains, encoder thresholds, turn counts, stopping
distances or control periods and label them calibrated values.

## Hardware verification and calibration

Treat design pin assignments separately from verified physical wiring.
Confirm actual hardware and relevant datasheets before actuation.

Check these documented conflicts:
- GP0/GP1: right encoder assignment conflicts with UART console or
  `CRASH_REPORT=1`.
- GP28: left encoder assignment conflicts with the optional Robo Pico
  VBAT-sense jumper if fitted.
- GP8/GP9: left motor pins conflict with the supplied I2C0 configuration
  if that currently disabled driver is enabled.

Do not enable a peripheral or diagnostic mode without checking every
affected pin and shared resource.

Confirm encoder resolution, decoding convention, polarity, gearing,
wheel dimensions and vehicle geometry before deriving motion targets.
Calibrate speed, distance, turns, PID and stopping behaviour using
repeatable measurements.

Verify sensor/encoder voltage compatibility, power arrangements, motor
and servo requirements, PWM resource allocation and physical wiring.
Do not infer electrical safety from a successful build.

Servo selection, sensor thresholds, timing parameters and hump-height
methods require evidence. Keep development defaults clearly labelled
as unvalidated.

## Verification and reporting

For each authorised change:
- Run the relevant build and tests when permitted.
- Report the exact commands, build profile, tested commit or working
  tree, results and limitations.
- Distinguish static inspection, compilation, host simulation,
  on-target RTOS tests and physical hardware tests.
- Never describe unrun tests as passing.
- Do not treat simulation or compilation as proof of physical accuracy,
  safe operation or predictable SMP timing.

Use `testing/README.md` conventions:
- Define stable subsystem, integration and system test IDs.
- Record expected results, actual outcomes and saved evidence.
- Preserve previous results; append new runs.
- Identify the exact tested revision and any uncommitted changes.
- Record hardware calibration conditions and measured errors.
- Use `videos/README.md` for hardware-test video evidence.

Existing placeholder logs and documentation claims are not new
verification evidence. Do not flash or conduct hardware tests without
authorisation.

## Git discipline

Before editing, inspect the current branch, HEAD, working-tree status
and relationship to local `main`. Preserve existing user changes.

Work on the authorised Buddy 2 branch. If a new branch is authorised,
use the `codex/` prefix unless the user specifies another name.

Do not switch, reset, rebase, merge or delete branches, or fetch/pull,
without authorisation. Do not discard work.

Do not stage, commit or push without explicit approval. Approval to
edit files is not approval to commit or publish them.

Do not commit or push `reference_docs/` to GitHub without explicit
approval. Do not commit credentials, generated build outputs or
unrelated files.

At completion, show the relevant diff, verification results and
remaining uncertainties.

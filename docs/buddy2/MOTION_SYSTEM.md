> Current location: normal repository `C:/Users/Hp/Documents/GitHub/INF2004-Embedded-Systems`,
> branch `buddy2-Motor-and-encoder-control`. The 2026-10-14 development report
> below is historical; see the current handoff at the end and the official
> test record for subsequent runs. Source C/H files remain in subsystem2.

# Buddy 2 M3–M7 software and integration handoff

Implemented and verified on 2026-10-14 on branch
`buddy2-Motor-and-encoder-control`, starting HEAD
`9e40a56d55955291c54957c459e02af1480bc31e`.
Local main at inspection: `2886fa8a0118185674f59964ab832d7c4e3ceabb`.
The verification covers the uncommitted working tree, including new files.
No branch, index, commit, push, merge, firmware flashing or powered test occurred.

Software is ready for team integration review. It is **not a fully linked,
hardware-validated integrated release**. The shared GPIO dispatcher, Vehicle
Controller connection, calibration and target timing remain prerequisites.

## Implementation and ownership

Production/test file names below are relative to `app_program/subsystem2/`;
this document and the other technical narratives are in `docs/buddy2/`.

| Files | Responsibility |
|---|---|
| `enc_capture.c/.h` | Paired x4 decoding, polarity, health latch and coherent count snapshots |
| `drv_encoder.c/.h` | Explicit input setup and subscription through external GPIO-bank hooks |
| `motion_bus.c/.h` | One copied request, admission backpressure, stop/cancel and copied status |
| `task_motion.c/.h` | Actual micro T-Kernel startup, synchronization, timing and sole motor owner |
| `wheel_loop.c/.h` | Calibration validation, estimators, speed slew, independent PIDs and M2 motor driver |
| `move_plan.c/.h` | Relative travel, wheel-progress correction and differential turn targets |
| `motion_run.c/.h` | Command lifecycle, freshness, deadlines, stopping and durable results |
| `motion_api.h` | Provisional Vehicle Controller entry points |
| `test_drive.c/.h` | S2-U57 through S2-U97; entirely injected encoder/fake-output fixtures |
| `verify_motion.py` | Local host, analyzer, style-subset and ARM object verification |
| `MOTION_SYSTEM.md` | This handoff, test mapping, limits and proposed integration |
| [Archived ownership report](../../testing/subsystem2/logs/2026-10-14_motion_ownership.txt) | Byte-preserved historical source/ownership/Git audit |
| [Evidence manifest](../../testing/subsystem2/logs/2026-10-14_evidence_manifest.md) | Exact archive paths and hashes for the relocated verification logs |

Original verification logs now reside in `testing/subsystem2/logs/`, using the
manifest's date-prefixed `.txt` names. The [official test record](../../testing/subsystem2/TEST_LOG.md)
contains S2-U01–U97 definitions and imported preliminary working-tree outcomes.
The unchanged verification script still writes future outputs in subsystem2;
its recipe input `m2_make_recipes.txt` remains here. This relocation changes no
historical result and does not execute tests or establish commit-verified passes.

Existing files changed: `subsystem2.c` adds motion init/start hooks after the
retained demonstration objects/task; `subsystem2.h` includes `motion_api.h`;
`subsystem2_test.c` adds the new runner and moves all existing pure groups onto
its native test worker without changing their assertions. Producer implementation,
pool, semaphore, messages and inherited tests are preserved. Accepted M1/M2
implementation files have not been changed.

The actual Make wildcard in `build_make/mtkernel_3/app_program/subdir.mk`
discovers `app_program/*.c` and `app_program/*/*.c`. All new C files are flat in
subsystem2, so no Makefile change is needed. Documentation/scripts do not enter
the firmware build. Host hardware code is excluded; normal TEST=0 excludes test
bodies. TEST=1 exercises synthetic outputs only and does not enable the motor
service or install an encoder IRQ handler.

## Implemented and partially connected features

- M3: GPIO event-to-x4-count capture, independent polarity, sticky invalid-edge
  and count-limit faults, coherent observation timestamps and generation checks
  are implemented and host-tested. Input setup compiles for ARM. **Real edge
  delivery is blocked on the external GPIO-bank subscription implementation.**
- M4: `subsystem2_init()` creates objects and a dormant task; start runs the demo
  and motion task. Startup remains UNCONFIGURED with no GPIO/PWM setup. One task
  owns both motors. No Vehicle Controller currently supplies configuration or
  commands. Kernel synchronization code compiles but has not run on target.
- M5: Signed continuous wheel speeds, acceleration/deceleration slew, actual
  elapsed time, fresh paired measurements, independent corrected M1 PID loops,
  overspeed checking, M2 duty limiting, guarded reversals and no-progress
  deadlines are implemented with fake-backend tests.
- M6: Relative forward/reverse odometry, speed/deceleration cap, wheel-progress
  correction, cancellation, finite timeout and post-braking completion checks
  are implemented and tested without a physical plant.
- M7: In-place left/right 90-degree and left/right 180-degree requests, calibrated
  per-wheel target scaling, normalized progress coordination and both-wheel
  completion checks are implemented. No physical angle accuracy is established.

## Provisional integration API and command policy

Only `motion_api_*` functions in `motion_api.h` are integration entry points.
Transitive internal definitions are available for configuration/types; motor
backend functions and owner state are not team actuation APIs. Other buddies
must not call the driver, access owner objects or manipulate PWM/GPIO directly.

The Vehicle Controller owns mission choices, arbitration and permission to
enable motion. Buddy 3 supplies line-following intent to that controller;
Buddy 4/5 supply sensing/stop/avoidance events. Buddy 2 executes the arbitrated
request and reports results. This interface is a candidate requiring agreement.

1. After subsystem startup, prepare a complete measured `wheel_loop_config_t`
   and a persistent `drv_encoder_port_t` context. Submit `motion_api_configure`.
   The configuration is copied, validated and immutable. It cannot be replaced
   after configuration/enable. Changing calibration requires an unpowered
   reviewed restart and explicit reconfirmation, not editing the active copy.
2. An explicit authorised procedure calls `motion_api_enable`. It subscribes the
   encoders and arms M2 to BRAKE, then enters SETTLING. READY requires valid
   feedback and count-stable dwell on both wheels. Configuration alone never
   enables hardware; no calibrated values are supplied automatically.
3. `motion_api_submit` copies a `move_plan_command_t`. IDs are nonzero and new
   accepted IDs strictly increase without wrap. Finite requests require positive
   cruise magnitude, signed nonzero distance for travel, and a deadline.
4. Continuous requests specify signed left/right speeds in mm/s. Refresh the
   SAME active ID with a strictly newer `issued_us` before the previous expiry.
   New IDs cannot preempt active commands. Stop/cancel, wait and acknowledge
   first. Continuous expiration brakes and reports EXPIRED; finite expiry is
   a latched TIMEOUT fault. Repeated stops cannot extend the stopping deadline.
5. `QUEUED` means copied intake, not acceptance or physical completion. Check
   `last_request_id`, `last_request_issued_us` and `b_request_accepted` for owner
   processing, including failed refreshes. Backpressure returns BUSY; retry
   without assuming intake occurred. A rejected refresh leaves the previous
   target/deadline intact. An idle rejected command retains a REJECTED result.
6. A terminal result remains until `motion_api_ack(terminal_id)` is processed.
   Admission stays blocked meanwhile. Fault cause is separately retained in
   `fault_result`, so an emergency cannot overwrite an earlier terminal result.
   Acknowledgement does not clear FAULT or re-enable outputs.
7. `motion_api_stop` is a normal immediate brake request, independent of an
   occupied ordinary request slot but using the short polling mailbox mutex.
   `motion_api_cancel(id)` applies to that active/pending/in-flight ID.
   Callers must handle BUSY. Normal stop/cancel are not emergency paths.
8. `motion_api_estop` is task/handler callable and uses a dedicated latched kernel
   flag plus wake, without the command/status mutex. The owner checks it before
   ordinary requests and before backend prepare/each output. It brakes/disarms
   both motors; no runtime reset exists. ERROR from posting requires the team's
   independent power-removal policy. A software stop is not proof of standstill.
9. `motion_api_status` copies coherent status; QUEUED here means copy succeeded.
   `timestamp_us` is publication time; `measurement_timestamp_us` is feedback
   observation time. `measurement_max_age_us` defines validity. The status API
   invalidates stale speed/standstill flags at read time. Position is historical
   per-wheel micrometres from the enable baseline, not an x/y/heading estimate.

All task APIs use TMO_POL mutex acquisition. Callback contexts must remain alive.
One controller arbitrates calls; requests are bounded copies, not pointers to
caller-owned command storage. All-zero configuration is unusable and non-driving.

## Algorithms, units and safety limits

Encoder polarity is applied once by the decoder. Estimation reuses M1 integer
scaling: decoded counts per wheel revolution and loaded circumference in µm.
Counts/position are signed int64; requested/measured speeds are signed int32
mm/s. Speed truncates toward zero; position retains fractional µm. No new speed
filter was invented. Stationary snapshots receive current observation time,
not last-edge time. Invalid transitions, reset generations, stale/gapped/backward
time, count/scale overflow and overspeed fail closed.

Setpoint slew uses µm/s internal velocity and a fractional remainder, with
explicit mm/s² acceleration/deceleration. Direction changes ramp through zero
on a separate update. M2 then requires BRAKE, its measured/reviewed guard and
fresh count-stable dwell before electrical reversal. Integral/ramp buildup is
reset while held in reversal. Reversal cannot wait beyond `stop_timeout_us`.
No encoder progress under a nonzero target faults at `progress_timeout_us`.
That deadline must accommodate the lowest usable speed and encoder resolution.
It is not current sensing, a comprehensive stall detector or a current limit.

Finite distance goals are relative to both accepted-command wheel positions.
The speed cap is floor(sqrt(2 * deceleration_mm_s2 * remaining_um / 1000)),
limited by requested cruise speed. A fixed 32-step integer square root is used.
Normalized progress mismatch reduces the leading wheel speed by configurable
gain and correction limit; it never raises speed beyond the cap. This is wheel
coordination, not line sensing or heading feedback. Slew remains bounded; if a
wheel coasts/brakes outside tolerance the manoeuvre fails rather than being
reported complete. Very small distances/speeds can be unachievable at mm/s and
encoder quantization; tolerance and timeout must be selected from measurements.

For an in-place 90-degree turn, each nominal wheel arc is track_width * π / 4;
180 degrees uses track_width * π / 2. π uses rational 355/113 and explicit
per-wheel parts-per-million calibration. Opposite wheel signs distinguish left
and right. Arithmetic overflow or an arc not exceeding tolerance is rejected.
Track width means effective lateral separation, not longitudinal wheelbase.

Position tolerance on BOTH wheels starts braking. DONE requires subsequent
fresh speed validity and unchanged encoder counts over `stop_dwell_us`, with
speed at/below the configured threshold; final position is rechecked. Slip,
encoder disconnection with plausible constant levels, motion between samples,
missed full quadrature cycles and external pushing cannot be ruled out by this
software. Independent power isolation and hardware supervision remain necessary.

Validation checks representation and consistency, not physical plausibility.
No wheel dimensions, gearing, polarity, gains, dwell, stop distance, acceleration
or turn scale in the tests is a robot calibration. Failed output confirmation
is visible separately; software never claims an unconfirmed brake is successful.

## GPIO, PWM, RTOS and dispatcher assumptions

- Encoders: left GP7/GP28, right GP0/GP1. Capture uses one GPIO_IN sample per
  dispatched event, state `(A << 1) | B`, positive raw cycle 00→01→11→10→00.
- Motors reuse M2: GP8/GP9 are PWM4 A/B, GP10/GP11 are PWM5 A/B. M2 checks clock,
  pins and slice aliases. Backend requires the verified 125 MHz clock profile.
  Period = ceil(clock/requested_frequency), TOP = period−1, divider=1;
  actual frequency is clock/period and never above the request or 20 kHz.
  Duty is capped. LOW/LOW means BRAKE, HIGH/HIGH means COAST on this board.
- GP0/GP1 must be free of UART console/crash reporting. GP28 VBAT jumper must
  be physically disconnected for the encoder. ADC pin initialization must not
  claim GP28. I2C0 GP8/GP9 and I2C1 GP6/GP7 conflicts must be resolved. The new
  backend rejects enabled I2C, configured UART conflicts, occupied outputs or
  unsupported mux overrides rather than changing shared configuration.
- GPIO-bank IRQ13 is shared. Buddy 5's integrated folder is empty; its retained
  standalone ultrasonic code polls echo. This is not permission to claim the
  bank permanently. No new handler is installed, replaced or enabled here.
- External reserve/enable/release hooks must preserve other users, route BOTH
  edges on all four encoder inputs, acknowledge only owned events, baseline
  before enabling, run on the agreed core, and quiesce callbacks on release.
  Enable requires wheels stationary and power inhibited. Dispatcher failures
  must latch stop via `motion_api_estop`, not publish apparently fresh counts.
- SMP capture uses the actual `T_SPLOCK` / `ISpinTryLock` / `ISpinUnlock` APIs;
  local IRQ masking is used only in UP. Failed acquisition latches a sensor
  fault. No PID, output operation, printing or network call occurs in capture.
  The kernel's underlying atomics/locks require target latency assessment.
- Time comes from `tk_get_otm`, converted from monotonic milliseconds to µs;
  the inspected tick is 10 ms. Period must be a multiple of that tick and at
  least 10 ms; PID/estimation use actual elapsed time. Duplicate/early wakes do
  not run PID. Command wakes do not postpone the next control deadline.
- Motion task candidate priority 15, SMP affinity TP_PRC2, static aligned stack
  4096 bytes. These are provisional resource choices, not measured scheduling
  or stack guarantees. Team review must resolve priorities, affinity, edge rate,
  jitter, service WCET and emergency-stop maximum latency before powered use.
- Emergency work precedes normal command/control processing and avoids ordinary
  queues. Nominal response is owner dispatch/wake plus bounded application stop
  work, with the periodic wake as fallback. No hard response bound is verified:
  higher-priority tasks, kernel locks, interrupts and output hardware still
  require measurement. A stalled scheduler cannot be fixed by a software flag.
- Additional production resources: one task, one priority-inheritance mutex,
  two event flags and static storage. Kernel-object allocation failures prevent
  readiness; task start returns the actual micro T-Kernel start error.

## Executed verification and limits

Python used:
`C:/Users/Hp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`.
From repository root, exact final arguments were:

```
python -B app_program/subsystem2/verify_motion.py --style
python -B app_program/subsystem2/verify_motion.py --label final --analyze
python -B app_program/subsystem2/verify_motion.py --label final --arm --strict
git diff --check
```

The script contains the exact compiler locations and saves expanded commands.
Host GCC 14.2.0: C99 default and O2 + fshort-enums, both with `-fanalyzer`,
Wall/Wextra/Werror/pedantic/conversion/sign-conversion/shadow/strict-prototypes/
missing-prototypes/format=2/double-promotion/undef checks. **95/95 groups PASS
in EACH profile: all original 54 plus 41 new groups.** No existing assertion or
test group was removed/weakened. Milestone gates: M3 59, M4 61, M5 68, M6 80,
M7 initial 88; final safety regressions bring the total to 95.

ARM GNU 10.3.1: all 19 subsystem2 translation units compiled in each combination
of SMP=0/1 and APP_TEST=0/1 (76 compilations). The script reuses the professor's
saved Make recipe and changes only input, profile defines and output/dependency
paths, all within Buddy 2. Strict application warning checks passed with zero
diagnostics; supplied headers are system includes for that check. Existing
inherited test/demo translation units retain normal template compiler options.
These are **object compilations, not a full Make build or link**.

SDK 2.2.0 was not found in the configured environment/tool installation; the
installed Pico SDK is 1.5.1. The accepted prior integrated build remains blocked
by the template's SDK 2.2.0 versus installed 1.5.1 USB compatibility mismatch.
Make was not invoked in this run because its outputs/parse-time writes can leave
Buddy 2. No UF2/ELF or complete firmware size is claimed. No dependency installed.

Compiler stack estimates in the final SMP/TEST profile: motion_run_init 976 B,
wheel_loop_init 776 B, task_motion_cycle 384 B, task_motion_entry 64 B; largest
new test frame 1384 B. Existing pure test call chains also leave insufficient
initial-task margin. These are individual frames, not complete call-chain or
interrupt bounds. All Buddy 2 pure TEST=1 groups therefore use a static 8 KiB
worker, not the template initial task's 1024+128 B stack. It is one-shot with a
30-second test-only timeout; it uses fake outputs, self-deletes on completion,
and avoids forcibly terminating a worker that could hold logging/kernel locks.
On timeout it reports failure and leaves its completion object alive. Target
test execution, stack watermarks and timeout sufficiency have not been tested.

Style subset passed on 17 new C/header files: ≤80 columns, spaces, no trailing
whitespace, EOF markers and distinct first-eight module filename characters.
Production public bodies precede private definitions; synchronization and checked
signed arithmetic were manually reviewed. This is not certified Barr-C compliance.
Necessary conventions retained: binary32 `motion_pid_value_t` (no standard C99
float32_t), vendor INT/UINT/ER/ID types and MMIO interfaces, the task entry exit
return after tk_ext_tsk, function-pointer callbacks and synthetic test fixtures.
Existing professor/demo style is deliberately unchanged. clang-format,
clang-tidy and cppcheck were not available on PATH; no results are claimed.

No known unresolved high-severity portable-code defect was identified in the
final review. Shared interrupt integration, full linking and unmeasured target
timing/electrical/calibration behavior remain release blockers. Host tests do
not demonstrate SMP scheduling, physical current, motion accuracy or safety.

## New test mapping (all synthetic)

| ID | Expected property |
|---|---|
| S2-U57 | Baseline and complete positive quadrature cycle |
| S2-U58 | Independent polarity and stationary observation timestamps |
| S2-U59 | Invalid transition latches unhealthy |
| S2-U60 | Null, generation and calibration rejection |
| S2-U61 | Count overflow latches without wrapping |
| S2-U62 | Copied mailbox, capacity and terminal acknowledgement gate |
| S2-U63 | Pending-command cancellation despite occupied slot |
| S2-U64 | Signed speed/distance, independent loop and explicit brake |
| S2-U65 | Stale feedback disarms both outputs |
| S2-U66 | Generation reset and null-path rejection |
| S2-U67 | Negative overspeed fails closed |
| S2-U68 | Motor polarity, timing, PID and nonfinite-config consistency |
| S2-U69 | Failed output stops both; failed brake remains unconfirmed |
| S2-U70 | Zero crossing, reversal guard and count-stable dwell |
| S2-U71 | Relative forward targets and independent wheel arrival |
| S2-U72 | Reverse targets and wrong-way rejection |
| S2-U73 | Leading-wheel reduction, integer deceleration cap, overshoot |
| S2-U74 | Uninitialised planner, enum, range, future and zero-life rejection |
| S2-U75 | Position/timestamp overflow and INT32_MIN distance |
| S2-U76 | Delayed DONE, durable result, exact acknowledgement and next ID |
| S2-U77 | Reverse executive completion |
| S2-U78 | Cancellation brake/result and replay rejection |
| S2-U79 | Continuous refresh, replayed refresh rejection and exact expiry |
| S2-U80 | Finite exact timeout disarms |
| S2-U81 | Emergency latches, acknowledgement cannot re-enable |
| S2-U82 | Repeated stops cannot extend stopping deadline |
| S2-U83 | Left/right 90/180-degree target signs and arc values |
| S2-U84 | Unequal calibrated turn arcs and normalized leading-wheel correction |
| S2-U85 | Turn overflow, zero scale and unrepresentably small arc rejection |
| S2-U86 | Both turn wheel targets and standstill required |
| S2-U87 | Post-braking turn drift becomes OVERSHOOT |
| S2-U88 | Unhealthy capture becomes SENSOR fault |
| S2-U89 | Failed status publication cannot reopen admission prematurely |
| S2-U90 | Early/duplicate wakes skip PID; backward time faults |
| S2-U91 | No-progress timeout brakes despite repeated nonzero requests |
| S2-U92 | Caller edits cannot change copied control calibration |
| S2-U93 | New publication cannot refresh stale feedback |
| S2-U94 | Request acceptance/rejection metadata and brake confirmation |
| S2-U95 | Emergency cause and earlier durable terminal coexist |
| S2-U96 | Moving wheel cannot hold reversal indefinitely |
| S2-U97 | Injected GPIO→decoder→estimator→executive→PID→fake PWM and fault stop |

## Proposed shared changes — not applied

1. Team-owned candidate `app_program/gpio_dispatch.c` and `.h`: register IRQ13
   once through the professor's interrupt mechanism, preserve other pin users,
   implement drv_encoder_port_t reserve/enable/release, route both encoder edges,
   handle cross-core ownership, lost-event faults and bounded callback release.
   Agree this with Buddy 5 and all GPIO IRQ consumers before implementation.
2. `TEAM.md`: agree encoder/motor pins, VBAT jumper, I2C/UART restrictions, PWM
   slices 4/5, GPIO IRQ/core ownership, object budget, task priority/affinity,
   calibration owner, emergency path and provisional motion contracts.
3. Vehicle Controller location is not implemented/agreed. Candidate
   `app_program/vehicle_controller.c/.h`, with separately approved `app_main.c`
   init/start calls, would own arbitration and call motion_api_* after startup.
   No app_main.c/app.h change is needed for the already implemented S2 hooks.
   Buddy 1/3/4/5 interface updates need their respective owners' agreement.
4. [Buddy 2 test records](../../testing/subsystem2/TEST_LOG.md) and
   [archived evidence](../../testing/subsystem2/logs/2026-10-14_evidence_manifest.md)
   were organised under the professor's convention during the separately approved
   folder cleanup. Uncommitted historical runs remain preliminary; formal
   commit-verified reruns and on-target/hardware execution are still outstanding.
5. SDK/tool environment: provision the required SDK 2.2.0 and dependencies with
   approval; then run the unchanged documented Make workflow. No Makefile,
   kernel or device change is proposed to bypass the current SDK mismatch.

## Calibration and future hardware verification checklist

Before enable, inspect exact board/wiring, encoder supply/output levels, GP28
jumper, UART/I2C users, GPIO/slice aliases, independent power isolation and
motor supply/stall-current/thermal limits. Verify brake/coast using Robo Pico
Table 3, startup glitches and error fallback using instruments with motor power
inhibited first. See accepted `MOTOR_DRIVER.md` for evidence and electrical limits.

Measure each wheel's x4 counts per loaded revolution, gearbox inclusion once,
wheel circumference, encoder/motor polarity, effective track width and separate
turn compensation. Establish max usable speed, minimum resolvable speed, PID
gains/integral bounds/duty ceiling, acceleration/deceleration, reversal guard,
stopping tolerance/speed/dwell/timeout and no-progress deadline. Reconfirm all
calibration after a parameter change. No synthetic fixture should be copied as
a robot setting without independent evidence.

After separate authorisation: scope GPIO edge rate/loss, snapshot contention,
cross-core visibility, stale/reset faults, loop WCET/jitter, CPU load, stack
watermarks and emergency latency under USB/Wi-Fi/demo load. Test both directions,
unequal wheels, low speed, saturation, overspeed, reversals, brake/coast,
encoder unplugging and output-failure handling with safe power removal available.
Measure repeated travel/turn error, slip, stopping distance and post-brake drift.
Exercise controller TTLs, arbitration, rejection, busy retry, cancellation,
acknowledgement and emergency events from all relevant buddies. Preserve videos
and official test records only in separately approved locations.

## Normal-repository handoff — 2026-10-15

Final organisation is performed in the original repository and branch, starting
at HEAD `0482b352c4d30e8e95831ae8ff931c8952dbfeb9`. No branch/worktree is
created or switched. The previous worktree was already archived at the user's
request; its snapshot `7215e6055c9539535e19c434433c398a7750ba5e` is inspected
read-only to preserve documentation and later genuine test records. All 38 C/H
files, script and recipe match its implementation apart from line endings.

### Requirement coverage and release boundary

| Responsibility | Actual implementation | Remaining verification/integration |
|---|---|---|
| Two motors, signed effort, PWM, brake/coast, guarded reversal | drv_motor and drv_rp2040; synthetic U43–U56 | Electrical sequencing/current, polarity and guard measurements |
| Two encoders, x4 polarity/count/invalid/overflow handling | motion_quad and enc_capture; U13–U18/U57–U61 | Real dispatcher hooks, edge rate/loss and SMP target tests |
| Signed wheel speed and distance | motion_estimator; U19–U25/U38–U39 | Loaded circumference, decoded counts/rev and physical error |
| Two PIDs, saturation and anti-windup | motion_pid and wheel_loop; U26–U42/U64–U70 | Gains, timing, plant stability and target WCET |
| Continuous differential speeds | MOVE_PLAN_SPEED, same-ID TTL refresh, acceleration/deceleration | Controller arbitration and line-following integration |
| Forward/reverse travel and straight correction | Relative wheel goals, leading-wheel reduction, both-wheel completion | Distance/stopping/slip calibration |
| Left/right and U-turns | Opposite wheel arcs and separate compensation; U83–U87 | Effective track width and angle accuracy |
| Stop/cancel/deadlines/latched emergency faults | motion_run, motion_bus, dedicated safety flag | Actual scheduling/stop latency and independent power-removal policy |
| Lifecycle and status | Real task init/start, copied status and immutable configuration | Team API agreement; no automatic configuration/enable caller |

Portable logic is implemented. Kernel/hardware paths are not proven by host
tests. Priority 15, TP_PRC2 affinity, 4 KiB motion stack and 8 KiB TEST worker
remain provisional until team scheduling review and measured stack margins.
Emergency work avoids ordinary queues, but no hard response bound has been
measured. Fault acknowledgement does not clear the latch; recovery requires an
unpowered reviewed restart, not automatic re-enable.

### Demonstration retirement — coordinated integration task

`subsystem2_start()` already starts the real motion task, alongside the retained
producer. Removing only producer startup would strand Buddy 3's consumer in
`tk_rcv_mbx(...,TMO_FEVR)` (`app_program/subsystem3/task_consumer.c:25`).
Removing SAMPLE_MSG, sem_slots or mpf_samples would also break that consumer's
compiled references and Buddy 4's monitor (`task_monitor.c:92–93`), plus S2-U01/02.
The pool, semaphore, declarations and producer are therefore the minimum
unchanged compatibility chain for the currently started examples.

Buddy 3 must retire its consumer startup and mailbox/pool return dependencies
while retaining the real navigation task. Buddy 4 must retire synthetic batch,
statistics, semaphore/pool monitoring and related tests. Shared startup/tests
and TEAM.md then require coordinated updates. Only afterwards can Buddy 2
remove producer_task, s2_make_value, sample types and compatibility objects.
This is not a pre-merge code blocker for a non-actuating compatibility-preserving
merge; it is required before the final robot configuration claims no synthetic
producer activity. No other buddy's code was changed.

### Unresolved shared contracts

The Vehicle/Mission Controller must arbitrate Buddy 1 remote requests, Buddy 3
line/turn intent, Buddy 4 motion/terrain events and Buddy 5 stop/avoidance intent.
Buddy 2 alone owns motor outputs. motion_api_* remains provisional. Agree IDs,
TTL/refresh, BUSY retry, cancellation/acknowledgement, emergency callers and
configuration lifetime. Week 6 FR17 requests raw left/right encoder counts;
current motion_run_status_t exposes speed and historical wheel position, not
raw counts. Adding raw count telemetry is a team-reviewed interface decision,
not an implemented capability. No public contract was silently changed.

The external GPIO-bank dispatcher must supply reserve/enable/release, both
edge directions on GP7/GP28/GP0/GP1, one simultaneous input sample, owned-bit
acknowledgement, agreed core affinity and quiescent release. Pin/PWM reservations
need TEAM.md agreement. GP0/GP1 cannot use UART/crash reporting; GP28 requires
checking the VBAT jumper and ADC ownership; GP8/GP9 conflict with I2C0, and GP7
with I2C1. Current disabled I2C/ADC-pin setup is not permanent ownership proof.

### Manual Git readiness

The user explicitly approves retaining root AGENTS.md and reference_docs.
Those files are unchanged. Six historical integrated ARM/Make verification
logs have been moved from build_make into testing/subsystem2/logs, with original
bytes and hashes preserved. See the [relocation index and integrity record](../../testing/subsystem2/logs/2026-10-15_final_verification.md#integrated-arm-log-relocation--2026-10-15).

The final working-tree diff against locally available main
`2886fa8a0118185674f59964ab832d7c4e3ceabb` contains no build_make changes.
The committed-only diff still contains the old log additions until the user
commits the relocation. The previous scope-only merge blocker is resolved by
this approval and file move; full linking and shared integration limitations
remain. No remote freshness or conflict-free merge is asserted. Review and
commit the final file changes manually; no staging, commit, push, merge, fetch
or history rewrite is performed by this organisation task.

The installed SDK remains 1.5.1; the documented build requires SDK 2.2.0
(or the README's separately tested 2.3.0). Full firmware linking remains
unverified. Provision a compatible SDK/tool environment separately; do not
patch the professor's Makefiles. No firmware was flashed or motor energised.

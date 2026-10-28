# Buddy 2 Motion Control — Technical Guide

This is the authoritative current Buddy 2 guide. Production algorithms and
motor ownership are implemented in the professor's micro T-Kernel project.
Host verification and ARM object compilation are distinct from full linking,
on-target execution and physical validation. No startup path enables motors.
The Vehicle Controller owns mission decisions; shared interfaces remain provisional.

## Initial physical straight-line operation 2026-10-28

The user explicitly authorised the existing integrated system to attempt
three-second held-button runs with provisional motion settings. This section
supersedes the older unconfigured-startup notes below. There is no alternate
application or trial build. From the root: `make -C build_make S4_IMU=1 -j8`.

`app_main.c` supplies development configuration and permits power, without a
boot movement request. Release GP20 after boot, then hold GP20 to request START
through the existing controller and S2 API. Release requests PAUSE; after three
seconds PAUSE is requested regardless of continued holding. Release and press
again for another request. A blocked press is consumed. GP21 posts S2 emergency
before provider waits and latches until reboot. Use the programmable GP20/GP21
buttons, not the board's full-speed motor buttons. The three-second request
limit is not a guarantee of physical stopping time.

DEVELOPMENT explicitly labels estimated geometry/PID/motor settings. It does
not claim CONFIRMED calibration. Motor and PID output ceilings are 25 percent;
only nonnegative continuous speeds with leases at most 300 ms are accepted.
Finite travel/turns are rejected. The adapter requires local buttons and blocks
network START/RESUME from bypassing the local limit. Invalid encoder transitions,
stale feedback, no-progress faults, line loss and emergency handling remain.

User wheel diameter is 62 mm (approximate circumference 194779 um). Remaining
assumptions: 1000 counts/revolution, both motor/encoder polarities NORMAL,
unused track width 150 mm, P=0.01 and I=D=0. Forward request 50 mm/s, steering
correction 15 mm/s. Actual speed/distance are uncalibrated estimates. IR starts
with black=HIGH on all channels; USB IRraw shows actual levels for feedback.

Line and motion remain mandatory. IMU acquisition uses confirmed GP16/GP17,
but IMU/obstacle status are optional for the initial flat, clear track. Missing
or faulty optional providers are not converted into healthy samples. GP15
servo signal is confirmed; obstacle acquisition remains unconfigured pending
voltage, supply and endpoint checks. This run provides no obstacle avoidance
or collision/hump guarantee. Zero required_sensors preserves the prior
all-provider policy for other configurations.

Build/host results are appended in the integration log. Physical direction,
encoder scale, button latency and stopping distance still need real runs.

Historical implementation/verification details remain in MOTOR_DRIVER.md,
MOTION_FOUNDATION_VERIFICATION.md and the evidence indexed by TEST_LOG.md.
The superseded combined guide was preserved byte-for-byte as a historical
artifact before consolidating its repeated handoffs; it is not current guidance.

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
| `testing/subsystem2/verify_motion.py` | Local host, analyzer, style-subset and ARM object verification |
| `MOTION_SYSTEM.md` | Authoritative current technical guide |
| [Archived ownership report](../../testing/subsystem2/logs/2026-10-14_motion_ownership.txt) | Byte-preserved historical source/ownership/Git audit |
| [Evidence manifest](../../testing/subsystem2/logs/2026-10-14_evidence_manifest.md) | Exact archive paths and hashes for the relocated verification logs |

Host-only tests, the single verification runner and its saved recipe live in
`testing/subsystem2/`. Firmware-discovered tests remain flat beside source to
preserve the professor's unchanged TEST=1 discovery. They are the authoritative
implementations, not copies under testing/. Test definitions/results have one
index: [TEST_LOG.md](../../testing/subsystem2/TEST_LOG.md).

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
   invalidates stale speed/standstill/count flags at read time. Position is historical
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
- GPIO-bank IRQ13 is owned by the Phase 2 shared dispatcher on core 1. Buddy 2
  supplies reserve/enable/release hooks and never installs a competing handler.
  Buddy 5 has software-only planning and no live acquisition API. Its future
  echo client must coordinate with the dispatcher; no pin/IRQ is claimed here.
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

## Initialization, rollback and recovery contract

All subsystem init calls must finish before any start call. Initialization is
single-threaded and pre-publication. The existing semaphore/pool declarations,
SAMPLE_MSG layout, producer code and cross-buddy demonstration dependencies
are preserved. No ownership transfer or runtime teardown has been introduced.

- subsystem2_init creates its demonstration semaphore/pool, then motion objects.
  On failure it deletes only successfully created, still-private objects and
  returns the original error. Motion initialization follows the same rule.
- After complete rollback, initialization may be retried. Repeated successful
  initialization returns E_OBJ without changing IDs or allocating objects.
- A failed deletion retains its positive ID and blocks another initialization
  attempt with E_OBJ. The original creation error is retained for the failed
  call. Recovery from incomplete rollback is reboot only; no resource ID is lost.
- subsystem2_start rejects calls before successful initialization and repeated
  attempts. The first attempt preserves professor ordering: producer, then motion.
  If motion startup fails, the producer may already be running. Published
  demonstration objects are retained; never delete them beneath other buddies.
  Start retry/runtime restart is unsupported and requires reboot recovery.
- The shared app_start_task helper only reports its existing return value;
  Buddy 2 cannot certify producer scheduling when that helper masks a start
  error. Correcting that shared helper is a separately approved team change.
- Failed owner wait/clock operations brake, release the encoder subscription,
  publish fault status where possible and exit the owner. Fault recovery is
  reviewed, unpowered/reboot only. A successful event post is not physical proof.

U99–U103 exercise motion retry and repeated init; U121–U134 exercise subsystem
rollback, one-shot startup, retained IDs and fake-resource validation. All fakes
are host-only; they do not emulate real scheduling, SMP visibility or IRQ latency.

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

## Shared integration gates

Agree dispatcher ownership, GPIO/PWM allocation, task resources and the final
motion contracts through TEAM.md and the respective owners before integration.
No shared file is changed by this correction pass. Dispatcher release must
quiesce core-1 callbacks before retiring their context; its current void hook
requires that guarantee. Cross-core dispatcher operations are rejected.
Both-edge coalescing/loss and maximum edge rate require target measurement.
Required transition rate is CPR*abs(speed_mm_s)*1000/circumference_um; implement
with checked arithmetic. The speed quantum is
1000*circumference_um/(CPR*elapsed_us) mm/s. CPR includes x4/gearing once.

Buddy 1 telemetry conversion must resolve signedness/range and invalidity.
The motion status now exposes the accepted signed count pair as described below.
Buddy 1/shared telemetry must agree signed range, validity and generation/reset
semantics before consuming it. Never relabel rounded distance as counts.
Phase 2 maps measured forward odometry to Buddy 3 while preserving source age.
Buddy 4 now reads signed motion feedback independently with explicit traction
trust. Hardware calibration/trust remains pending. Different clock origins
cannot be reconciled by unit division.
Buddy 5 avoidance requests go through the Vehicle Controller, not motor drivers.

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

## Verification and repository organisation

Run from repository root:

```text
python -B testing/subsystem2/verify_motion.py --style
python -B testing/subsystem2/verify_motion.py --analyze --label UNIQUE_HOST
python -B testing/subsystem2/verify_motion.py --arm --strict --label UNIQUE_ARM
```

The runner defaults to testing/subsystem2/logs and refuses evidence overwrite.
It hashes production/test/runner/recipe files by repository-relative path and
records base HEAD, dirty status, compiler commands and UTC start. Host execution
has a bounded timeout (default 5 s; --host-timeout range >0 to 60 s), returning
nonzero 124 with a TIMEOUT diagnostic. Compiler invocations are bounded at 120 s.
The timeout self-check intentionally records EXIT 124; guards and mutation checks
intentionally record rejection. These are not acceptance-suite failures.

Generated executables, objects, dependency and stack-usage files use a temporary
directory under testing/subsystem2/build, covered by the existing 'build' ignore
rule. Each private subdirectory is removed on success/failure; the empty ignored
parent can remain for later/concurrent runs. No shared ignore rule changed.
Host-only test_rtos uses actual task_motion, drv_encoder and subsystem2 sources
with kernel/GPIO/motor fakes. Kernel ABI names and host main/setjmp/longjmp follow
the external test conventions; compact fake types and the scenario dispatcher
are test-only Barr exceptions. The style check is a subset, not certification.
Existing 95 pure groups remain; 37 host adapter/lifecycle groups extend coverage.
Seven temporary mutations check selected regression assertions; they do not
constitute exhaustive mutation testing or physical safety proof.

Compiler paths remain explicit local prerequisites in the runner. ARM checks
reuse the saved unchanged Make compiler recipe and compile 19 source files for
SMP=0/1 and TEST=0/1. They are object checks, not a full Make build or link.
The installed SDK 1.5.1 does not establish compatibility with the required 2.2.0.
After separate tool provisioning, use the professor's documented build_make
workflow: make -j8 and make TEST=1 -j8. No workaround to shared rules is applied.

All results, current source hashes, historical logs and archive hashes are
indexed only in [TEST_LOG.md](../../testing/subsystem2/TEST_LOG.md). S2-U01/U02
remain target tests; host results do not establish those outcomes. Historical
working-tree results are preliminary, not clean commit-verified evidence.
Keep production C/H in app_program/subsystem2, host tools/evidence in testing/
subsystem2 and technical guidance here. Do not relocate tests used by TEST=1
without an approved source-discovery change. Do not commit generated artifacts.


## Signed encoder feedback (2026-10-15)

`motion_api_status()` retains its existing polling mutex/coherent-copy contract.
Appended fields are `encoder_count[2]` (left/right signed int64 decoded x4
transitions), `encoder_timestamp_us`, `encoder_generation` and `b_encoder_valid`.
Encoder polarity is already applied once; values are not scaled by gearing or
reconstructed from rounded distance. Counts can reach INT64_MIN/MAX without
narrowing. Decoder limits latch a fault instead of wrapping.

Only a healthy, matching-time, matching-nonzero-generation pair accepted by both
wheel estimators replaces the cached count pair. A rejected/partially processed
sample never mixes old/new wheel counts. The first baseline may have valid
counts while speed validity remains false. The sample timestamp is kernel
operating-time us (10 ms resolution here), taken at capture observation; it is
neither the last edge nor the status publication time. Stationary observations
are fresh; early command wakes/STOP publications do not re-stamp old counts.

Unconfigured/unobserved status has false validity and generation zero; zero
counts alone do not establish standstill. Freshness uses measurement_max_age_us:
exact age limit is accepted, stale/future/clock-error reads invalidate the copy,
preserving historical counts/time/generation. Re-reading a cached accepted sample
can recover read-time validity if the consumer clock succeeds again; an actual
source/owner fault stays latched. BUSY/UNAVAILABLE do not provide a new snapshot;
check API result before using output. Faults retain history but clear validity.
There is no live count reset/fault recovery. Reboot starts a new origin; generation
is acquisition-local, not a globally unique boot/session identity.

ISR feed and capture snapshots retain the existing interrupt-safe spinlock;
owner status publication and reader copies retain the existing inheritance
mutex. No new lock, task, allocation, GPIO initialization or actuation path is
introduced. The shared dispatcher and all other buddies remain unchanged.
Host fakes check lock balance and release ordering, not real SMP memory ordering
or latency. Resource sizes and actual evidence are recorded in the existing
[TEST_LOG.md](../../testing/subsystem2/TEST_LOG.md).

Shared integration must explicitly consume these fields and agree with Buddy 1
on signed telemetry, validity, units and reboot handling; those contracts and
controller changes are outside this Buddy 2 pass. Mission movement remains
interlocked on main's existing provider requirements. Full firmware linking
still requires the documented toolchain/SDK; object checks are not physical
validation. Existing motor, PID, estimation, planning and fault-reset policy are
preserved; malformed zero/mixed generation capture now fails before acceptance.

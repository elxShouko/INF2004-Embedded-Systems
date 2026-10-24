# Buddy 5 obstacle subsystem — status and design notes

The [2026-10-21 Tribunal implementation record](TRIBUNAL1_IMPLEMENTATION.md)
contains the current representation audit, private acquisition/driver changes,
measured memory evidence, regression results and remaining hardware gates.
Older placeholder tables below are historical; they do not override the current
integration status or establish physical calibration.

## Buddy 5 - Adaptive Ultrasonic Scanning and Obstacle Profiling

The current integrated micro T-Kernel implementation is described first.
Later standalone Pico SDK sections are retained as historical design notes;
the root Make build does not compile standalone files in `src/` and `include/`.

---

## Current integrated status (2026-10-16)

`subsystem5_init/start` now create/start a real core-1 owner at priority 20,
with a static 4096-byte stack, inheritance mutex and dedicated fault flag.
Startup is unconfigured and performs no GPIO/PWM activation. Explicit pre-start
`s5_live_configure` copies immutable acquisition/policy configuration and shared
IRQ hooks; no default mission caller supplies it. Wiring and calibration must
be confirmed before opt-in. Failed initialization cleans up only its own
objects, preserves original errors, and prevents retries over unreleased handles.
Failed task start may retry; successful restart/live fault reset are unsupported.

`s5_acquire` is the bounded hardware-independent engine. `drv_s5` owns TRIG
GP26 as SIO, ECHO GP6 through the existing core-1 dispatcher, and **only confirmed
GP15/PWM7B** as the currently supported servo signal. GP15 remains unverified
physical wiring; other servo pins are rejected rather than assigned implicitly.
PWM7 ownership, GP14 aliases, ADC pin configuration and the template 125 MHz
clock are checked before claiming pins. PWM4/5 and motor GPIO are untouched.
Divider 125 yields a 1 MHz servo counter; caller-supplied period/min/max pulse
widths determine frequency and angle mapping. Mechanical endpoints, mounting
orientation and settling time must be measured before confirmation.

Echo rise/fall timestamps are captured by a short callback and copied under
same-core IRQ exclusion. Private backend access is core-1 only; cross-core
status readers use the inheritance mutex, never volatile/local masking as an
SMP lock. ISR service timestamps are not hardware-latched edge timestamps;
latency/jitter must be measured. Missing/coalesced edges, stuck-high trigger
admission, echo deadlines, clock regression, cancellation and scan motion fail
closed. Close releases only the owned subscription and disables servo output.
Status retains source capture time and historical values with validity cleared;
failed POL reads leave caller storage unchanged. A separate fault latch does
not depend on status publication or telemetry queues. Reboot is required after
a latched fault. GPIO/servo disable latency still needs target measurement.

Forward monitoring runs independently of motor enable, avoiding circular startup
readiness. Scanning requires fresh S2 READY/standstill feedback after STOP/ACK;
it aborts on motion or unavailable stationary evidence. Five coarse positions
and the existing bounded fine-angle generator feed the existing profile logic.
Actual echo capture times, not task read times, timestamp samples. If the ISR
finishes after the current cycle's clock read, consumption waits until the next
cycle; acceptance remains bounded by the original echo deadline. The
[HC-SR04 datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
specifies a >=10 us trigger and recommends cycles over 60 ms. Monitoring and
scans enforce a configurable interval >60000 us, counted conservatively from
the previous echo completion; settling uses non-blocking deadlines. The 10 us
trigger uses the BSP delay with IRQs enabled; preemption stretch is unmeasured.
The same datasheet specifies 5 V operation. Existing 3V3 wiring, ECHO level
shifting and actual module variant must be resolved before connecting/operating
it. No physical range accuracy is claimed.

Copied `s5_live_status` and fault APIs are connected to the controller.
Forward freshness gates motion independently of full-scan diagnostics.
An unconfigured/stale/faulty provider inhibits movement. Scan consistency limits,
pulse/range conversion and source-age budgets have no production defaults.
Software tests use labelled fixtures, never production synthetic ranges.

### Configurable bypass and moving recovery

The software now implements an eight-action route in the existing planner and
central controller: turn outward 90 degrees, travel laterally, turn forward,
travel beyond the bounded obstacle depth, turn inward, return the lateral
distance, turn forward, then search forward a bounded distance for the line.
Right routes mirror left routes. All travel is forward; no blind reverse is
inserted. The final search may finish stationary if the line is already found.
It is a bounded nominal-line reacquisition, not an unbounded search of arbitrary
branches. Failure to find the line within the distance/time bounds faults.

The legacy range-only s5_build_obstacle_plan still returns
CLEARANCE_UNVERIFIED with zero actions. New s5_route_plan accepts explicit
s5_route_config_t and scan-specific s5_clearance_t. It never treats minimum
side ranges or estimated angular width as certified free space.

Required measured/configured inputs, with no production defaults:
- Half-length about the axle (maximum front/rear extent), half-width including
  wheels/body, sensor forward offset, clearance margin, total route tracking/
  angular-error envelope and stopping-distance bound at the configured speed.
- An enclosing obstacle half-width and depth bound, lateral extra distance,
  forward search distance, speed, per-action and total route deadlines.
- Three independently verified FREE rectangles covering the outward, forward,
  and return/search corridors, in the stopped-scan frame (x forward, y left,
  micrometres). Include blind/rear sectors and full-height body clearance.
- Full raw-timer scan ID, evidence observation time and expiry. The evidence
  must postdate the completed scan and remain valid through the route deadline.
  Its verification attests scene/coverage/occlusion/error assumptions; expiry
  alone does not establish static-scene safety.
- Original-line evidence: the initial axle/heading matches the original line
  within the tracking envelope, and the search corridor contains no competing
  eligible line. A raw LINE_FOUND event cannot establish line identity.

The conservative swept radius is half-length + half-width + clearance +
tracking error + stopping bound. This L1 radius encloses rotations without
floating-point underestimation. Every entire axis-aligned swept segment must
fit its corresponding rectangle. Claimed free boxes intersecting the bounded
obstacle are rejected. Coordinates use int64 intermediates and bounded input
ranges before conversion to int32 motion distances. Existing profiles are
positive-right; the planner explicitly converts to its positive-left frame.

s5_live holds a valid stopped scan in S5_RUNTIME_HOLD instead of fabricating a
plan or faulting merely because clearance is unavailable. It continues actual
forward monitoring. The controller's optional vehicle_runtime_clearance API
copies a certificate under its existing inheritance mutex while stopped;
E_OK means copied for owner validation, not motion acceptance. No default
caller or automatic certificate producer exists. A lone HC-SR04 scan cannot
observe all required free space; numeric calibration alone does not resolve
that observability limitation. A validated coverage/environment model or
additional evidence source is still required for autonomous authorization.

The controller freezes an accepted plan, sends only one finite S2 command
at a time, waits for matching owner admission and terminal ID, repeatedly
ACKs until READY/standstill, and never refreshes finite moves. Translation
waits for a forward sample triggered after the preceding action completed;
a recent echo from an older trigger is insufficient. Unexpected modes,
wrong/rejected terminals, stale providers, certificate expiry, scan changes,
timeouts, PAUSE, emergency and ambiguous barcode events fail closed. Only
S2 actuates motors. Turns rely on the certified swept envelope; forward
translation additionally retains fresh proximity stopping.

S3 stays in BYPASS during the detour and RECOVERY for search. A centred,
raw/filtered-consistent, fresh LINE_FOUND for the current search stops motion;
the controller accepts S3's actual PAUSED-on-found behavior. It waits for the
matching stopped terminal/ACK/standstill, sends idempotent s5_live_recovered
for the exact scan ID, observes rearmed monitoring, then explicitly requests
TRACKING. Recovery clears the old scan-complete marker so the next obstacle
cannot reuse the previous profile. Mode waits, each action and total route
are bounded. No live reset or automatic retry of a failed route is provided.

Geometry/clearance/line-identity evidence is absent in the current repository.
Movement remains inhibited for this route. Host fixtures verify the complete
software sequence; they are not physical certificates or calibrated settings.

Standalone sections below describe legacy bench code, not live RTOS interfaces.
Their simulated stop/bypass callbacks never connect to the integrated mission.
Verification is recorded in testing/subsystem5/TEST_LOG.md.

## Integrated host-side validation and planning status

The integrated `s5_obstacle_logic` module now validates a caller-supplied scan
context before producing a profile. It rejects cancelled, failed, incomplete,
or non-stationary scans; missing validation limits; scans over their configured
duration or age; sample timestamps outside the scan window or out of order;
invalid ranges; and adjacent-range jumps over the supplied consistency limit.
Acquisition failure statuses distinguish trigger failure, echo timeout or
stuck-high/stuck-low conditions, and servo initialization or positioning
failure. These are pure-logic inputs; the new acquisition port supplies actual
trigger/echo/servo outcomes without asserting physical calibration.

Range comparisons can identify only a left/right **candidate**. They do not
prove vehicle clearance. Plan generation therefore returns
`S5_LOGIC_CLEARANCE_UNVERIFIED`, leaves the action count at zero, and marks the
plan non-executable. Reverse is likewise advisory metadata only. The caller
must not interpret the profile or candidate as permission to move.

The hardware-independent `s5_obstacle_runtime` policy layer checks configured forward
sample freshness, counts distinct proximity samples, requests a stop before a
full scan, keeps requesting stop until the stopped acknowledgement and
throughout scanning. Scan, bypass, and recovery requests remain asserted until
the policy observes their corresponding acknowledgement; a fault request
remains asserted while the fault state is latched. Stopped/bypass/line
acknowledgements must be fresh and timestamped after their corresponding
policy state begins. It fails closed on stop/scan timeouts, stale stopped
status, cancellation, movement during a scan, acquisition errors, or
unverified clearance. A valid scan is copied into a fixed-size status
snapshot. Its configuration has no production defaults; callers must provide
measured/approved values.

This policy remains pure and single-owner. The real s5_live task supplies
acquisition, source timestamps and bounded cross-task publication; readers use
s5_live_status rather than reading the private policy across cores. Stopped
status is mapped conservatively from S2's kernel clock into the raw timer
domain without renewing repeated measurements. Legacy stopped/bypass/line
acknowledgements retain main's freshness and post-request timestamp checks.
Timestamps do not replace request IDs: controller-owned recovery additionally
uses the exact full-width scan token. Completed malformed scans remain complete
but invalid, and forward faults revoke cached validity while retaining history.

The range-only planner remains non-executable. The configured planner and
controller described above can execute only with calibrated geometry and
verified scan-specific free corridors/original-line evidence. Main's legacy
pure-policy regressions and the integrated acquisition/recovery tests are both
preserved; colliding test IDs are mapped explicitly in TEST_LOG.md.
Physical use remains gated by the electrical/calibration/coverage requirements
below and separate hardware authorization.

## Unresolved integration placeholders

These placeholders make missing information visible without assigning
unverified hardware or inventing team agreements. `TBD` means unresolved and
must not be interpreted as a default, pin reservation, calibration value, or
approved interface.

| Item | Placeholder / current status | Required before implementation or vehicle use |
|---|---|---|
| Sensor and servo models | Module is visually identified as HC-SR04, exact revision unknown; a small servo is mounted beneath it, servo model unknown | Verify exact module revisions and datasheets/operating limits |
| Servo signal pin | `TBD — user now reports signal GPIO/model not confirmed` | Physically trace and verify before reserving or configuring a PWM pin; do not assume GP15 |
| Ultrasonic TRIG/ECHO pins | Current observed wiring: `GP26` = TRIG and `GP6` = ECHO | Verify wiring electrically before enabling a driver; keep GP26 GPIO/SIO-only because of the GP10/PWM5A alias |
| Ultrasonic supply / ECHO voltage | Current observed VCC is 3V3, GND is common; ECHO voltage and level shifting are unverified | Establish the actual module's supply and ECHO-high limits; do not connect an unverified ECHO level to RP2040 GPIO |
| Servo power and ground | `TBD — wiring and supply not electrically verified` | Verify supply/current, signal-level compatibility, and common ground |
| PWM resource / owner task | No verified servo signal pin or owning task; previous GP15/PWM7B assignment is unconfirmed | Resolve the GP15 claim in `TEAM.md` before PWM allocation; use exactly one reviewed owning task |
| Servo angle, pulse mapping, direction, settling time | `TBD — uncalibrated` | Measure on the assembled vehicle; define bounds and command-failure behavior |
| Sensor filtering, detection and range limits | `TBD — development values only` | Calibrate against repeatable measurements; document invalid, timeout, and overflow handling |
| Vehicle and sensor geometry / clearance | Vehicle width, sensor height/offset, beam geometry, centre angle, and approved safety margin are unmeasured; the sensor appears near front-centre and faces approximately forward when centred | Measure the assembled vehicle and sensor geometry; validate known obstacle distances and both-side clearances before setting a margin |
| Mission Controller stop/fault contract | `TBD — interface not agreed` | Define bounded, prioritized stop/fault delivery, acknowledgement, expiry, timeout, and recovery |
| Buddy 2 motion contract | `TBD — interface not agreed` | Define request/status types, ownership, arbitration, cancellation, completion, and failure results |
| Buddy 3 line-reacquisition contract | `TBD — interface not agreed` | Define request/event data, line-found/failure semantics, timeout, and ownership |
| Proposed obstacle integration | Buddy 5 proposes owning monitoring/handling, sending stop/fault and high-level bypass requests through the Mission Controller, and requesting line recovery from Buddy 3; Buddy 2 would return motion completion through the controller. This is not a team agreement. | Agree task/CPU ownership, message/event objects, data lifetime, acknowledgements, timeout ownership, cancellation, and fault behavior before implementation |
| Buddy 1 telemetry contract | `TBD — optional interface not agreed` | Specify payload and provider/consumer; use only the approved kernel network-service path |
| RTOS task structure and timing | `TBD — no Buddy 5 tasks implemented` | Agree task count, owners, priority/affinity, period/deadline, timeout, and overload behavior |
| Kernel object budget | `TBD — currently no Buddy 5 objects allocated` | Reserve required objects in TEAM.md/config review before creating them |
| Startup/error policy | `subsystem5_start()` returns `E_NOSPT`; current controller startup remains interlocked without Buddy 5 readiness/configuration | Preserve the readiness blocker; physical stop behavior still requires target verification and the approved fault contract |
| Integrated target build | `TBD — not run` | Run documented `make TEST=1 -j8` profile with configured WSL, Arm toolchain, and Pico SDK |
| On-target tests | `TBD — not run` | Build/flash only with authorization; record exact revision and test output |
| Physical calibration and system validation | `TBD — not run` | Obtain wiring authorization; record conditions, measurements/errors, results, and evidence |

---

## Features in the legacy standalone code (not integrated or hardware-validated)

### Ultrasonic Distance Measurement

- HC-SR04 trigger output on GP26
- HC-SR04 echo input on GP6
- Echo pulse timing
- Distance calculation in centimetres
- Valid-distance checking

### Ultrasonic Measurement Validation

- Multiple readings are taken before confirming an obstacle
- Invalid measurements are rejected
- Measurements are checked for consistency
- Repeated invalid readings are detected
- Echo timeout handling is implemented

### Continuous Obstacle Monitoring

The subsystem continuously monitors the distance in front of the robot.

Normal flow:

MONITOR  
→ possible obstacle detected  
→ VALIDATE  
→ obstacle confirmed

A single ultrasonic reading is not immediately treated as a confirmed obstacle.

### Adaptive Servo Scanning

The scanning system uses two stages.

#### Coarse Scan

A wide scan is first performed to locate the general direction of the obstacle.

Current coarse scan angles:

- 30 degrees
- 60 degrees
- 90 degrees
- 120 degrees
- 150 degrees

#### Fine Scan

After the approximate obstacle direction is found, a more detailed scan is performed around that area.

Current fine scan configuration:

- +/- 20 degrees around the detected obstacle
- 5-degree scan steps

This produces more detailed measurements around the obstacle while avoiding an unnecessarily detailed scan of the entire area.

### Servo PWM Control

The servo driver:

- uses Pico hardware PWM
- converts requested angles into PWM pulse widths
- moves the ultrasonic sensor to different scan positions
- returns the sensor to the forward-facing position after scanning

The physical servo GPIO has not yet been confirmed.

### Scan Sample Storage

Each ultrasonic scan measurement stores:

- servo angle
- measured distance
- whether the reading is valid

The stored measurements are used to generate the obstacle profile.

### Obstacle Profiling

The subsystem generates an obstacle profile containing:

- closest obstacle distance
- closest obstacle angle
- approximate lateral position
- approximate forward position
- estimated obstacle width
- left-side clearance
- right-side clearance

### Obstacle Width Estimation

Fine-scan measurements close to the nearest detected surface are grouped together.

The angular span and measured distance are used to estimate the approximate physical width of the obstacle.

### Left and Right Clearance Evaluation

The wider coarse scan is used to evaluate available space on both sides of the obstacle.

The subsystem checks:

- left-side clearance
- right-side clearance
- number of valid measurements on each side

This information is used when selecting a bypass direction.

### Bypass Direction Planning

The subsystem can produce:

- `BYPASS_LEFT`
- `BYPASS_RIGHT`
- `BYPASS_NO_SAFE_ROUTE`

If both sides appear safe, the side with greater clearance is selected.

If neither side has sufficient clearance, Buddy 5 reports a fault instead of attempting an unsafe bypass.

---

## Avoidance Action Planning

Buddy 5 generates a high-level action plan instead of directly controlling the motors.

Possible actions include:

- `STOP`
- `REVERSE`
- `TURN_LEFT`
- `TURN_RIGHT`
- `BYPASS`
- `SEARCH_LINE`
- `RESUME_LINE`

Example normal avoidance plan:

STOP  
→ TURN_LEFT  
→ BYPASS  
→ TURN_RIGHT  
→ SEARCH_LINE  
→ RESUME_LINE

Example when an obstacle is extremely close:

STOP  
→ REVERSE  
→ TURN_RIGHT  
→ BYPASS  
→ TURN_LEFT  
→ SEARCH_LINE  
→ RESUME_LINE

Buddy 5 only generates the required actions.

Actual motor control remains the responsibility of Buddy 2 and the Mission Controller.

---

## Line Recovery Planning

After the obstacle has been bypassed, the subsystem plans movement back toward the original line.

Example:

Bypass left  
→ turn back toward the right  
→ search for original line  
→ wait for `LINE_FOUND`  
→ resume normal line following

Buddy 3 remains responsible for physically detecting the line.

Buddy 5 waits for the `LINE_FOUND` notification before returning to normal obstacle monitoring.

---

## Buddy 5 State Machine

The current obstacle-handling state machine is:

MONITOR  
→ VALIDATE  
→ REQUEST_STOP  
→ SCANNING  
→ BUILD_PROFILE  
→ PLAN_BYPASS  
→ BYPASS  
→ LINE_RECOVERY  
→ MONITOR

A FAULT state is available from relevant failure conditions.

---

## Fault Handling

The subsystem currently handles:

- ultrasonic sensor timeout
- repeated invalid ultrasonic readings
- servo failure
- insufficient valid scan readings
- invalid obstacle profile
- no safe bypass route
- vehicle stop timeout
- bypass completion timeout
- line recovery timeout

Faults are reported to the Mission Controller rather than Buddy 5 directly controlling the vehicle.

---

## Mission Controller Interface

Buddy 5 currently provides callback interfaces for:

- vehicle stop request
- bypass request
- line-recovery request
- fault request

Buddy 5 also accepts notifications for:

- vehicle stopped
- bypass completed
- line found

This keeps obstacle sensing and planning separated from motor control and other subsystems.

---

## Telemetry / Status Information

Other parts of the system can retrieve:

- current Buddy 5 state
- current fault
- latest forward distance
- obstacle profile
- scan measurements
- selected bypass direction
- generated avoidance action plan

This information can later be provided to Buddy 1 for MQTT telemetry.

---

## RTOS Structure

The Buddy 5 logic is already separated into:

### Obstacle Monitor Logic

`obstacle_monitor_step()`

Intended for the high-priority periodic Obstacle Monitor Task.

Responsibilities:

- continuously monitor forward distance
- detect possible obstacles
- detect repeated sensor failures

### Obstacle Handling Logic

`obstacle_handling_step()`

Intended for the medium-high-priority Obstacle Handling Task.

Responsibilities:

- validate obstacles
- request vehicle stop
- perform adaptive scanning
- build obstacle profile
- select bypass direction
- generate avoidance plan
- coordinate bypass completion
- coordinate line recovery
- handle faults and timeouts

The actual µT-Kernel task wrappers are still pending because the project-specific µT-Kernel API/starter code is required.

---

## Hardware assignments in the legacy source (unverified)

The legacy standalone configuration contains GP26/TRIG and GP6/ECHO values.
The old source comment calling these connections “confirmed” was incorrect;
they are unverified demo values and are not used by the integrated Make build.

Current assembly observation: HC-SR04 VCC is connected to 3V3, GND to GND,
TRIG to GP26, and ECHO to GP6. The sensor's operating range at 3V3 and its
ECHO-high level at the Pico input have not been electrically verified or
measured with a meter/scope; no external level shifter/divider is confirmed.
Do not enable or connect the ECHO input to the RP2040 until the actual module
and signal level are checked.
GP26 is also PWM5A, which aliases Buddy 2's motor output on GP10. Buddy 5 must
use GP26 as ordinary GPIO/SIO only and must not configure its PWM mux or write
the PWM5A compare value.

The small servo is visually mounted beneath the HC-SR04 and is reported to
rotate it, but its model, signal pin, supply, and wiring are not confirmed.
The sensor appears near the vehicle's front-centre and faces approximately
forward when centred; its exact height, offset, centre angle, and beam
geometry are unmeasured. Earlier notes and `TEAM.md` claim GP15/PWM7B, but
the servo signal GPIO has not been physically traced. Treat GP15 as unresolved
and do not drive it until the pin, wiring, electrical limits, and shared PWM
ownership are verified. The legacy servo GPIO remains disabled at placeholder
255; the integrated µT-Kernel subsystem has no servo driver.

---

## Pending Work

### Hardware

- Verify the HC-SR04 module's supply requirements and ECHO voltage; add/verify level shifting if required
- Verify Pico W installation, 3V3/Robo Pico supply and common ground
- Trace and record the servo model, signal GPIO, supply and wiring
- Test HC-SR04 acquisition and servo movement only after electrical review and authorization
- Verify servo direction and centre position

### Calibration

- obstacle detection threshold
- reverse threshold
- safe-clearance threshold
- ultrasonic validation tolerance
- servo pulse widths
- coarse scan range
- fine scan range
- servo settling delay
- safety timeout values
- obstacle width estimation accuracy

### RTOS

- Wrap `obstacle_monitor_step()` in the actual µT-Kernel Obstacle Monitor Task
- Wrap `obstacle_handling_step()` in the actual µT-Kernel Obstacle Handling Task
- Configure final task priorities and scheduling

### Team Integration

- Mission Controller integration
- Buddy 2 motor-control integration
- Buddy 3 `LINE_FOUND` integration
- Buddy 1 MQTT/telemetry integration

### Final Validation

- obstacle detection tests
- profiling accuracy tests
- width estimation tests
- left/right clearance tests
- bypass-direction tests
- reverse-action tests
- fault-condition tests
- line-recovery tests
- full end-to-end obstacle avoidance testing

---

## Current Progress Summary

The integrated Buddy 5 calculation module validates caller-supplied scan
contexts, creates profiles, and deliberately refuses to emit an executable
route without clearance evidence. The new host-only runtime policy validates
configured forward-sample freshness, requires repeated distinct proximity
samples, requests a stop before full scanning, and fails closed for timeout,
cancellation, scan/motion faults, and unverified clearance. Host-only status
copies include scan timestamps, bounded samples, validity, profile, plan,
busy/completion state, and fault.

None of this code acquires real measurements or connects a thread-safe status
provider to the Vehicle Controller. The sensor/servo drivers and µT-Kernel
tasks, agreed stop/bypass/recovery communication, geometry-aware clearance
proof, target build, calibration, and physical validation remain outstanding.
The subsystem does not yet perform obstacle avoidance on the vehicle.

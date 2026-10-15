# Buddy 5 obstacle subsystem — status and design notes

## Buddy 5 - Adaptive Ultrasonic Scanning and Obstacle Profiling

This file describes the earlier standalone Pico SDK Buddy 5 demo. It is
separate from the integrated micro T-Kernel firmware under
`app_program/subsystem5/`; the root Make build does not compile the standalone
files in `src/` and `include/`.

---

## Current Status

**Buddy 5 is not yet operational in the integrated firmware.** The integrated
folder currently contains a host-testable scan/profile/planning calculation
module only. Its RTOS task startup reports `E_NOSPT` until hardware and team
interfaces are confirmed and implemented.

The standalone demo remains unverified: its servo GPIO is intentionally set
to 255, its callbacks simulate stop/bypass/line-found notifications, and it
does not execute actual vehicle motion. Neither that demo nor the current
host-only logic proves physical sensing, safe bypass, or line reacquisition.

The integrated `subsystem5_start()` currently returns `E_NOSPT`. The current
`usermain()` reports subsystem start errors and then continues startup, so
this placeholder is not a system-wide fail-safe or a vehicle stop mechanism.

The sections below describe the legacy standalone design unless explicitly
identified as integrated status. State names, callbacks, task names, and
telemetry listed there are not implemented public interfaces in the current
µT-Kernel subsystem.

## Unresolved integration placeholders

These placeholders make missing information visible without assigning
unverified hardware or inventing team agreements. `TBD` means unresolved and
must not be interpreted as a default, pin reservation, calibration value, or
approved interface.

| Item | Placeholder / current status | Required before implementation or vehicle use |
|---|---|---|
| Sensor and servo models | `TBD — exact part numbers not confirmed` | Record manufacturer and model; check datasheets and operating limits |
| Servo signal pin | `TBD — no GPIO claimed` | Confirm physical wiring, check TEAM.md pin ownership, claim before coding |
| Ultrasonic TRIG pin | `TBD — no GPIO claimed` | Confirm module wiring and claim an unowned pin |
| Ultrasonic ECHO pin and voltage | `TBD — no GPIO claimed; output level not verified` | Verify worst-case ECHO voltage and required level shifting before RP2040 connection |
| Sensor/servo power and ground | `TBD — not verified` | Confirm supply voltage/current, common ground, and Robo Pico power constraints |
| PWM resource / owner task | `TBD — no slice, channel, or task reserved` | Check pin-to-slice mapping and team PWM ownership; assign exactly one owning task |
| Servo angle, pulse mapping, direction, settling time | `TBD — uncalibrated` | Measure on the assembled vehicle; define bounds and command-failure behavior |
| Sensor filtering, detection and range limits | `TBD — development values only` | Calibrate against repeatable measurements; document invalid, timeout, and overflow handling |
| Bypass clearance, reverse distance, turn targets | `TBD — heuristic values only` | Agree with Buddy 2/controller and validate against vehicle geometry and stopping tests |
| Mission Controller stop/fault contract | `TBD — interface not agreed` | Define bounded, prioritized stop/fault delivery, acknowledgement, expiry, timeout, and recovery |
| Buddy 2 motion contract | `TBD — interface not agreed` | Define request/status types, ownership, arbitration, cancellation, completion, and failure results |
| Buddy 3 line-reacquisition contract | `TBD — interface not agreed` | Define request/event data, line-found/failure semantics, timeout, and ownership |
| Buddy 1 telemetry contract | `TBD — optional interface not agreed` | Specify payload and provider/consumer; use only the approved kernel network-service path |
| RTOS task structure and timing | `TBD — no Buddy 5 tasks implemented` | Agree task count, owners, priority/affinity, period/deadline, timeout, and overload behavior |
| Kernel object budget | `TBD — currently no Buddy 5 objects allocated` | Reserve required objects in TEAM.md/config review before creating them |
| Startup/error policy | `TBD — subsystem start returns E_NOSPT; usermain logs errors and continues` | Agree how the integrated vehicle is kept stopped or prevented from entering motion when any required subsystem fails |
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

The legacy configuration contains provisional HC-SR04 pin values:

- TRIG: GP26
- ECHO: GP6

The current wiring and ECHO voltage have not been confirmed. Do not connect
the sensor to RP2040 GPIO until its output voltage is verified as safe.

Servo GPIO remains unconfirmed and is intentionally configured as the invalid
placeholder value 255 in the legacy source.

---

## Pending Work

### Hardware

- Confirm the actual servo GPIO
- Obtain/install the Raspberry Pi Pico W on the Robo Pico
- Test HC-SR04 measurements on the real robot
- Test servo movement
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

The integrated Buddy 5 calculation module generates bounded development scan
angles, estimates an obstacle profile from supplied samples, and creates
high-level bypass/recovery action plans. Host tests cover off-axis profiles
centered on every coarse scan direction. Its scan orientation, thresholds,
and range-based bypass heuristic are development assumptions, not calibrated
safety limits. The off-axis projection regression is S5-U11.

The sensor drivers, µT-Kernel tasks, communication with the Mission Controller,
motion execution by Buddy 2, and line-found coordination with Buddy 3 are not
implemented. The subsystem therefore does not yet perform obstacle avoidance
on the vehicle. Hardware wiring/voltage confirmation, interface agreement,
target build/test, calibration, and physical validation remain required.
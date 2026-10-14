# INF2004 Embedded Systems

## Buddy 5 - Adaptive Ultrasonic Scanning and Obstacle Profiling

This branch contains the current implementation of the Buddy 5 obstacle detection, profiling, avoidance-planning and recovery subsystem.

Current development branch:

`buddy5-obstacle`

---

## Current Status

**Buddy 5 core software implementation is completed and compiling successfully.**

The subsystem is ready for:

- hardware validation
- calibration
- µT-Kernel task integration
- integration with the Mission Controller and other buddies

The remaining work depends mainly on hardware confirmation and team integration.

---

## Implemented Features

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

## Current Hardware Configuration

Confirmed HC-SR04 connections:

- TRIG: GP26
- ECHO: GP6
- VCC: currently connected to 3V3
- GND: GND

Servo GPIO:

- Not yet confirmed
- Currently intentionally configured as an invalid placeholder value to prevent accidental PWM output to the wrong GPIO

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

The following Buddy 5 core software has been implemented:

Detection  
→ validation  
→ vehicle stop request  
→ coarse scan  
→ fine scan  
→ obstacle profiling  
→ location/width/clearance estimation  
→ bypass-side selection  
→ STOP/REVERSE/TURN/BYPASS action planning  
→ recovery planning  
→ line reacquisition coordination  
→ resume normal monitoring

The latest implementation compiles successfully using the Raspberry Pi Pico SDK.

The remaining work primarily requires hardware testing, calibration, µT-Kernel integration and integration with the other team subsystems.
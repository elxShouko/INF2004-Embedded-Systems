# INF2004 Embedded Systems

## Buddy 5 - Obstacle Monitoring and Avoidance

This branch contains the current implementation of the Buddy 5 obstacle subsystem.

### Completed / Implemented

- HC-SR04 ultrasonic distance measurement
- Ultrasonic reading validation using repeated samples
- Ultrasonic timeout handling
- Repeated invalid-reading detection
- Servo PWM control
- Servo-based multi-angle ultrasonic scanning
- Storage of scan angle and distance samples
- Closest obstacle detection
- Approximate obstacle position estimation
- Approximate obstacle width estimation
- Left and right clearance calculation
- Bypass direction planning:
  - BYPASS_LEFT
  - BYPASS_RIGHT
  - BYPASS_NO_SAFE_ROUTE
- Buddy 5 obstacle state machine:
  - MONITOR
  - VALIDATE
  - REQUEST_STOP
  - SCANNING
  - BUILD_PROFILE
  - PLAN_BYPASS
  - BYPASS
  - LINE_RECOVERY
  - FAULT
- Vehicle stop request interface
- Vehicle-stopped acknowledgement handling
- Bypass request interface
- Bypass-complete handling
- Line recovery request interface
- LINE_FOUND handling
- Fault handling for:
  - ultrasonic sensor timeout
  - repeated invalid readings
  - servo failure
  - invalid scan
  - no safe bypass route
  - stop timeout
  - bypass timeout
  - line recovery timeout
- Mission Controller callback interface
- Status/profile getter functions for future telemetry
- Software separated into:
  - Obstacle Monitor logic
  - Obstacle Handling logic
- Standalone test harness for development without other buddies
- Current code compiles successfully using the Raspberry Pi Pico SDK

### Pending / To Be Completed

- Confirm actual servo GPIO connection
- Obtain/install the Raspberry Pi Pico W on the Robo Pico
- Physically test HC-SR04 readings
- Test and calibrate servo angles
- Calibrate obstacle detection threshold
- Calibrate safe left/right clearance threshold
- Validate obstacle position and width estimation
- Validate scan range and scan timing
- Implement actual µT-Kernel task wrappers
- Integrate with Mission Controller
- Integrate with Buddy 2 for motor stop/bypass completion
- Integrate with Buddy 3 for LINE_FOUND / line recovery
- Integrate obstacle data with Buddy 1 telemetry / MQTT
- Test fault conditions on real hardware
- Perform full end-to-end test:
  MONITOR -> VALIDATE -> STOP -> SCAN -> PROFILE -> PLAN BYPASS -> BYPASS -> LINE RECOVERY -> MONITOR

### Current Hardware Assumptions

- HC-SR04 TRIG: GP26
- HC-SR04 ECHO: GP6
- HC-SR04 VCC currently connected to 3V3
- Servo GPIO: not yet confirmed

### Notes

The current implementation contains the core Buddy 5 obstacle-detection, profiling, bypass-planning and recovery-coordination logic.

The subsystem is not yet considered fully complete because hardware calibration, µT-Kernel integration and team integration are still pending.
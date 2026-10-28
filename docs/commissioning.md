# Production robot startup and staged testing

This is the integrated micro T-Kernel application, not a replacement bench
program. The actual Vehicle Controller and Buddy 1-5 tasks run. Buddy 2 alone
owns motor outputs; the radio service alone owns CYW43/lwIP/MQTT calls.

The delivered profile is BENCH, with motor power permission false, motor
settings unconfigured, no automatic motor arm and no mission START. It enables
raw x4 encoder capture for hand rotation; this does not claim calibrated
speed, distance or forward polarity. Unknown servo pulse/timing parameters
leave ultrasonic acquisition disabled. The IMU and IR production owners supply
raw diagnostic readings. No target execution or physical calibration is implied
by a successful firmware link or host test.

## Wiring facts supplied by the team

| Hardware | GPIO |
|---|---|
| Left/right motor | 8/9 and 10/11 |
| Left/right encoder A/B | 7/28 and 0/1 |
| Left/right line and side barcode IR | 2, 4, 27 |
| LSM303DLHC IMU SDA/SCL | 16/17 |
| Ultrasonic TRIG/ECHO | 26/6; ECHO is the white wire |
| Single servo | 15 |

ECHO is professor-approved but not electrically measured. The GP28 VBAT-jumper
state is physically unverified; the professor instructed the team to ignore
it for this project. Firmware introduces no ADC/VBAT use on GP28. GP26 stays
GPIO/SIO; its PWM alias must not change the motor's PWM5A. Debug Probe UART
must not be attached to GP0/GP1: those are encoder inputs.

## One configuration source

Edit `config/robot_profile.h`, then rebuild and reboot. It is deliberately
explicit and suitable for one physical car; there is no general configuration
upload framework. `vehicle_profile_startup()` runs after all subsystem objects
and the GPIO dispatcher exist, before subsystem tasks start. For an autonomous
stage it calls the actual `vehicle_runtime_setup()`. Missing/invalid settings
leave movement blocked, print a profile error and retain diagnostics.

`prepared=1` in BENCH means the restricted bench path exists, not that motors,
PID, IMU height or the complete mission are calibrated. The immutable profile
cannot switch between bench and autonomous ownership while running.

| Stage | Profile settings | Behaviour |
|---|---|---|
| BENCH (0), delivered | `stage=VEHICLE_STAGE_BENCH` | Read sensors/counts; explicit bounded pulse only after separate motor configuration, ARM and PULSE |
| LINE (1) | `stage=VEHICLE_STAGE_LINE`, `mission.b_line_only=true`, `motion.profile.b_straight_only=true` | Existing closed-loop line following; side markings/junctions pause; turn requests rejected |
| BARCODE (2) | `stage=VEHICLE_STAGE_BARCODE`, both restriction flags false; `navigation.junction_enabled=true` | Existing barcode-to-junction manoeuvres and line recovery |

Both autonomous stages require `0 < run_limit_ms <= 30000`, a command TTL no
greater than 500000 us, fresh mandatory providers, power authorisation and an
explicit START. The run timer starts at the first accepted START/RESUME; pauses
do not reset it. Expiry or PC lease loss latches an emergency; reboot for the
next trial. These are engineering test limits, not measured vehicle limits.
No executable bypass route is enabled by this staged profile.

### Bench motor settings

Leave these unset until a separately authorised hardware procedure supplies
and reviews them: `bench.motor.pwm_hz`, `reversal_guard_us`, `max_duty`, both
`polarity[]`, `zero_mode`, resource confirmation and calibration confirmation;
`bench.effort_permille[2]`, `pulse_us`, `b_power_authorised`, `b_wheels_raised`.
The supplied 125 MHz clock is the platform clock, not a guessed calibration.
Both ARM and PULSE require fresh healthy encoders with unchanged counts for
at least 100 ms. This is a bench guard, not proof of ground traction or physical
standstill; raised wheels and operator checks remain required.

Motor configuration confirmation here concerns the electrical bench settings;
it does not confirm geometry or PID. Initial positive/negative electrical
polarity must be identified with wheels raised before assigning vehicle forward.

`pulse_us` must be a positive multiple of 10000 and no greater than 500000.
Effort requests are signed permille in -1000..1000; the production driver clamps
them to the explicitly supplied duty ceiling. Use a reviewed conservative
ceiling and pulse duration. There is one arm/pulse opportunity per boot. ARM
commands brake only; PULSE drives once. No live reversal, automatic repetition
or runtime fault reset is supplied. ARM expires after five seconds, pulse
expiry is checked in the motor-owning task, and the PC lease expires after
500 ms independently of controller printing and ordinary request queues.

The profile's `b_encoder_on_boot=true` selects hand-rotation diagnostics. With
it false, the MQTT ENCODERS command starts capture. Never use fake wheel
geometry to enable raw counts. Raw sign follows 00,01,11,10,00 positive, x4.

### First autonomous line-following profile

Before selecting LINE, supply measured/confirmed values for:

- `setup.motion.geometry`: counts per wheel revolution including x4/gearing
  once, loaded wheel travel, motor/encoder polarity, track width and speed limit.
- `setup.motion.motor` and both PID configurations: electrical limits, reviewed
  gains/bounds and confirmation. All-zero gains are not useful speed regulation.
- `setup.motion`: control period (10 ms tick multiple), sample limit, stop dwell,
  stop timeout, progress timeout, stop-speed threshold and overspeed limit.
- `setup.motion.profile`: acceleration/deceleration, position tolerance and
  straight correction settings. With `b_straight_only=true`, both turn scales
  may remain zero; all turn commands are rejected before division/planning.
- `setup.navigation`: measured IR polarity/validity and reviewed sensing,
  filtering, freshness/recovery and barcode-capture timing parameters accepted
  by the existing S3 API. Do not mark `valid_mask=7` before checking black/white
  response of all channels. Junction detection may stay disabled for LINE;
  the controller pauses when the calibrated side sensor sees black.
- `setup.obstacle`: servo pulse endpoints/period/settling, echo conversion and
  acquisition timing plus the existing policy's thresholds/timeouts. Set
  `setup.b_obstacle_requested=true` only with complete approved parameters.
  Runtime SENSORS explicitly activates the already configured GP15 servo and
  ultrasonic owner. A fresh forward range is required before driving.
- `setup.mission`: calibrated forward/correction speed, sensor age, command TTL,
  transition timeout, remote age, obstacle stop threshold and explicit power
  authority. Set `b_calibrated=true` only after the enabled behaviours are
  checked. `turn_mm_s` and `straight_um` may remain zero in LINE because finite
  barcode manoeuvres are disabled. Keep `route.b_calibrated=false`.

The IMU must have a healthy fresh baseline, but magnetic-heading and hump-height
calibration remain unconfirmed until independently measured. Height validity
is not replaced with an invented zero. Mandatory IMU/obstacle gates are retained;
a disconnected sensor is not treated as a harmless optional feature.

Begin on a clear, flat track section without barcodes, crossings, humps or
obstacles. Use explicit reviewed speed and run-duration limits. The same
Vehicle Controller, line status, motion executive, PID and real drivers perform
the run. Validate stopping before extending the duration or adding turns.

## Build targets and generated files

Use the README's configured WSL/Unix environment and Pico SDK:

```sh
cd build_make
make ROBOT=1 -j8
# USB diagnostics, real IMU, raw encoders, no MQTT commands:
# mtk3pico_smp1_usb_cdc_wifi_imu_robot.elf / .uf2

make ROBOT=1 NET=lwip MQTT=1 -j8
# mtk3pico_smp1_usb_cdc_wifi_lwip_mqtt_imu_robot.elf / .uf2
```

The network build needs a real local `config/wifi_secrets.h` and reachable MQTT
broker. No credentials are supplied or guessed. Compilation with the published
example header proves linkage only; an example-credential image cannot be used
for real network testing. Neither build automatically runs a motor test.
`ROBOT=1` rejects `TEST=1`, UART/crash-report console and single-core builds.
The existing normal and `TEST=1` targets remain available for regressions.

On this Windows machine the existing tools also build through MSYS Bash:

```powershell
# Run from build_make; no installation is necessary on this machine.
& 'C:\msys64\usr\bin\bash.exe' --noprofile --norc -c 'export PATH="/c/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin:/ucrt64/bin:/usr/bin:$PATH"; export PICO_SDK_PATH=../build/deps/pico-sdk; mingw32-make ROBOT=1 SHELL=C:/msys64/usr/bin/bash.exe -j8'
```

The earlier ARM DLL/MSYS shared-memory errors were sandbox launch restrictions;
the installed ARM GCC 10.3.1 works outside that restricted launch environment.
No compiler/SDK installation or download was required. Native Windows GNU Make
4.x uses a linker response file to avoid truncating the MQTT object list at
8191 characters; the WSL/Unix link command remains unchanged.

## Flashing later only with explicit authorisation

No flashing or motor action was performed during implementation. Before using
these commands, identify the exact profile and keep motor power inhibited.

Debug Probe: connect SWDIO, SWCLK and GND to the Pico W SWD pads, and supply
appropriate board power. From the repository root, this machine's installed
OpenOCD can program the ELF:

```powershell
& 'C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\openocd\openocd.exe' `
  -s 'C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\openocd\scripts' `
  -f interface/cmsis-dap.cfg -f target/rp2040.cfg `
  -c 'adapter speed 5000' `
  -c 'program {build_make/mtk3pico_smp1_usb_cdc_wifi_imu_robot.elf} verify reset exit'
```

For the network image substitute its exact ELF filename after real credentials
and the selected profile have been configured. This follows the official
[Raspberry Pi Debug Probe instructions](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html).
Do not halt or single-step a processor with motors powered: software deadlines
stop progressing while halted. SWD is for flashing/debugging, not a new command
protocol. USB BOOTSEL alternative: hold BOOTSEL while connecting the Pico's USB,
then copy the selected UF2 to RPI-RP2. It reboots automatically.

## Diagnostics without Wi-Fi

Connect the Pico W's own USB cable, find its USB CDC COM port and open a serial
terminal at 115200. The Probe's UART COM port is a different device and is not
used. The firmware repeats `[ROBOT]` diagnostics about once a second, including:

- stage, profile readiness, boot session, state, blockers, link and fault mask;
- raw IR DOUT bits, calibrated black mask, validity and source timestamp;
- raw signed x4 encoder counts as 64-bit two's-complement hex pairs, plus count
  validity, commanded wheel targets and motion fault result;
- bench intake/result, armed/running/used flags, requested effort, commanded PWM
  levels, retained last-pulse PWM and invalid encoder transitions; PWM is a command, not a measurement;
- last successful raw IMU sample and latest transport result, acceleration in mg,
  magnetometer in milli-gauss, phase and height validity; INT32_MIN means an
  unrepresentable diagnostic conversion, not a physical measurement;
- forward ultrasonic range/validity, configuration, scan/acquisition faults and
  estimated profile width. An unconfigured sensor is unavailable, not zero range.

API errors/validity/source ages must be checked before interpreting retained
values. Counts and diagnostics are available offline; powered commands are
intentionally unavailable without the verified MQTT control/lease path. Hand
rotation with motors inhibited can establish counts/revolution and raw sign.
IR black/white readings and a stationary IMU check require no fabricated scale.

## PC commands after network setup and hardware authorisation

Use the existing monitor and dashboard. Their existing paho-mqtt dependency
must be available; implementation does not install packages or contact a broker.
The binary telemetry/heartbeat and ordinary command formats remain unchanged.
The ROBOT-only extension is exactly five bytes: A1=ENCODERS, A2=ARM, A3=PULSE,
A4=LEASE, A5=SENSORS, followed by the current nonzero uint32 LE boot session.
No native C structure padding is sent. The boot session is an accidental-replay
challenge, not authentication. Malformed A-prefixed controls latch E-STOP.

Read the current session from USB output. Replace BROKER and SESSION below
with actual values; do not use an old session after reset.

```sh
# Main terminal: monitor real telemetry and keep the testing lease alive.
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --session SESSION --lease

# Separate terminal: request encoder capture if not already enabled at boot.
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --session SESSION --bench ENCODERS --seconds 2

# Only with a complete reviewed obstacle profile: may move the servo.
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --session SESSION --bench SENSORS --seconds 2

# Only with reviewed bench settings, raised wheels and power authorisation:
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --session SESSION --bench ARM --seconds 1
# Confirm quiet=1 and armed=1 on USB, then within five seconds:
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --session SESSION --bench PULSE --seconds 2

# After selecting/configuring LINE or BARCODE and checking provider readiness:
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --send START_RUN --seconds 2
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --send PAUSE --seconds 2
python testing/subsystem1/tools/s1_monitor.py --broker BROKER --send EMERGENCY_STOP --seconds 2

# Existing dashboard can also issue ordinary controls and show live telemetry.
python testing/subsystem1/tools/s1_dashboard.py --broker BROKER
```

Keep the lease monitor running before ARM or START, and throughout the trial.
The two-second command monitor is not the motor pulse duration. MQTT publishing
only confirms transport intake: check USB owner result=1, armed/running flags,
valid counts and faults. result=2 means rejection. Repeated ARM while armed
stops the bench test. ARM/PULSE are not automatically retransmitted on reconnect.
Use PAUSE for a controlled stop, E-STOP for a latched emergency. Power removal
remains the independent fallback if software does not respond physically.
Never use retained messages for robot control.

### Progress to barcodes and the complete course

For BARCODE, first measure turn scales/targets, stopping and junction origins,
enable junction detection, remove straight/line-only restrictions, and validate
reacquisition. Install the actual four-letter map with the existing dashboard
or monitor's `--map` option; see `docs/barcode-demo.md`. No map is present at boot.

Full-course readiness additionally requires calibrated hump-height/trusted
motion evidence, obstacle scanning geometry and a real validated supplier of
scan-specific swept-path/original-line clearance. This implementation does not
invent that supplier, mark traction trusted or enable executable bypasses. Full
mission accuracy, SMP timing, stopping latency and resource limits still need
physical system tests. The minimum-bit review remains firmware-wide work;
compact bench flags and explicit wire fields do not establish full compliance.


## Software verification and delivery, 2026-10-28

Tested branch `buddy2-Motor-and-encoder-control`, HEAD
`993927ce2a0a258f7dc7ab91838194a22d45eda0` plus the uncommitted changes listed below.
No staging, commit, push, board access, flashing, physical movement or real MQTT
connection was performed. All hardware SYS-03..10 cases remain NOT RUN.

| Verification | Result and limits |
|---|---|
| GNU Make normal, TEST=1, ROBOT=1, ROBOT=1 NET=lwip MQTT=1 | All four ARM GCC 10.3.1 builds/link/UF2 conversions pass. MQTT used the published example header solely for compilation; it is not an operational network image. |
| Normal integration, default and optimized | 85 integration groups, 12 bench/protocol groups, retained startup checks, eight mutation detections and S5 policy/acquisition/live/MMIO checks pass per profile. |
| Robot integration, default and optimized | Seven startup/session groups, 12 bench/protocol groups, seven actual S2 owner groups, three S5 activation groups and three raw IMU copy groups pass per profile. |
| S2 existing regression, default and optimized | 98 pure and 43 adapter checks per profile; timeout/host guard checks and eight mutation detections pass. |
| S1 existing host verifier including robot command queue | All suites pass; robot netbin reports 74 checks, zero failures. No real broker was contacted. |
| S3 production timing verifier | All ten existing logical/algorithm/10 ms host suites pass. S3 production code is unchanged. |
| S4 existing core/runtime/transport checks | Functional checks pass; existing imu_core.c narrowing and imu_packed.c type-limit warnings remain visible. These two warning categories are nonfatal in this host run; this is not a clean strict full-S4 build claim. |
| Existing INT-86 hump-wire boundary | Strict host compile and execution pass. |
| Additional ARM warning check | Eleven selected changed production modules pass with strict conversion/prototype/format warnings as errors, supplied headers treated as system headers, and TM_MQTT=0 explicitly defined for the USB-only profile. This is not a full Barr-C certification. |

Commands used from the repository root (Python is the installed bundled Python;
host GCC is the existing `C:/msys64/ucrt64/bin/gcc.exe`):

```sh
python testing/integration/verify_integration.py --label robot_final_normal --output-dir EXTERNAL_DIRECTORY
python testing/integration/verify_integration.py --robot --label robot_delivery --output-dir EXTERNAL_DIRECTORY
python testing/subsystem2/verify_motion.py --analyze --label robot_regression_1 --output-dir EXTERNAL_DIRECTORY
python testing/subsystem1/host/verify_windows.py --log EXTERNAL_DIRECTORY/s1_acceptance.json
python testing/subsystem3/verify_production_timing.py --gcc C:/msys64/ucrt64/bin/gcc.exe --log EXTERNAL_DIRECTORY/s3_robot.json
```

The S3 evidence JSON and S4/INT-86 records contain the exact executed commands.
The Make commands and exact installed Windows tool paths are given above.
Raw logs, compile-only objects, input hashes and build manifests are kept outside
the repository, not added as ad hoc test records:

`C:/Users/Hp/.codex/visualizations/2026/10/28/01a18098-f8c8-75d1-ac37-345075e31ddc`

Evidence: `build_verified/firmware_results.json`, `strict_robot_results.json`,
`verification/robot_final_normal_{default,optimized}.log`,
`verification/robot_delivery_{default,optimized}.log`,
`verification/verify_robot_regression_1_*`, `verification/s1_acceptance.json`,
`verification/s3_robot.json`, `verification_remaining_final/remaining_results.json`
(S4 checks only; its earlier INT-86 attempt failed to link), and
`verification_remaining_final/int86_final.json` (corrected INT-86 compile/run pass).
Earlier failed attempts are retained rather than overwritten or described as passing.

Firmware SHA-256 from the final source-matched build:

| Profile | UF2 SHA-256 |
|---|---|
| normal | `8a5c52009b43b08b3c207060e5cdff51145058d78eb6580301646791f0050f5b` |
| selftest | `ce39dd252876c8493f29ba9c0f8bf9b39974429c3675a3c68ff2752ff6d70178` |
| robot_usb | `9cfe7d116a84ab2298dc0b5c07534f7e53aee86c3d7b28f82aab020f7a5a1973` |
| robot_mqtt_example | `e1b35241d91a3a80abc9ce98337ce008488f49bec0286583b501da44dee6ced8` |

### Complete changed-file inventory

`M` means modified, `A` means a new intended source/document, and `D` means the
two previously authorised duplicate helper-file deletions. No generated raw
logs are in this change. Production changes affect shared startup/controller/build,
S1 protocol transport, S2 motor owner, S4 copied diagnostics and S5 activation;
S3 production implementations are reused unchanged. All remain uncommitted.

```text
A	app_program/subsystem2/bench_motion.c
A	app_program/subsystem2/bench_motion.h
A	app_program/vehicle_profile.c
A	app_program/vehicle_profile.h
A	config/robot_profile.h
A	docs/commissioning.md
A	testing/integration/test_commission.c
D	testing/subsystem2/logs/minbits_firmware_20261028_after/build_driver.sh.txt
D	testing/subsystem2/logs/minbits_firmware_20261028_before/build_shell.sh.txt
M	.gitignore
M	README.md
M	TEAM.md
M	app_program/app_main.c
M	app_program/subsystem1/s1_codec.c
M	app_program/subsystem1/s1_codec.h
M	app_program/subsystem1/s1_net.c
M	app_program/subsystem2/enc_capture.c
M	app_program/subsystem2/enc_capture.h
M	app_program/subsystem2/motion_api.h
M	app_program/subsystem2/move_plan.c
M	app_program/subsystem2/move_plan.h
M	app_program/subsystem2/task_motion.c
M	app_program/subsystem2/test_drive.c
M	app_program/subsystem4/imu_runtime.c
M	app_program/subsystem4/imu_runtime.h
M	app_program/subsystem5/s5_live.c
M	app_program/subsystem5/s5_live.h
M	app_program/vehicle.c
M	app_program/vehicle.h
M	app_program/vehicle_rt.c
M	build_make/makefile
M	build_make/pico_rp2040.mk
M	testing/integration/TEST_LOG.md
M	testing/integration/test_vehicle.c
M	testing/integration/verify_integration.py
M	testing/subsystem1/TEST_LOG.md
M	testing/subsystem1/host/test_net_bin_host.c
M	testing/subsystem1/host/verify_windows.py
M	testing/subsystem1/tools/s1_monitor.py
M	testing/subsystem1/tools/s1_wire.py
M	testing/subsystem2/TEST_LOG.md
M	testing/subsystem2/test_rtos.c
M	testing/subsystem2/verify_motion.py
M	testing/subsystem4/TEST_LOG.md
M	testing/subsystem4/host/runtime_main.c
M	testing/subsystem5/TEST_LOG.md
M	testing/subsystem5/host/test_obstacle_runtime.c
M	testing/system/TEST_LOG.md
```

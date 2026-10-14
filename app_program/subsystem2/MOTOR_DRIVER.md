# Buddy 2 M2 motor driver â€” implementation and verification

Implemented 2026-10-14. Internal Buddy 2 interfaces only. No automatic hardware
initialization, no demonstration-task connection, no encoder IRQs and no RTOS
tasks were added. No powered test or physical motor actuation was performed.

## Files and architecture

- drv_motor.c/.h: portable configuration validation, effort conversion, stop
  selection, per-wheel direction history, reversal state and fault handling.
- drv_rp2040.c/.h: explicitly bound Cytron Robo Pico backend using professor
  BSP GPIO/PWM facilities plus localized missing register operations.
- test_motor.c/.h: fake GPIO/PWM backend and S2-U43..U56 tests.
- subsystem2_test.c: only test-header/runner additions; original tests retained.

Short module names keep their first eight characters distinct under Barr-C.
No shared/public team interface is changed; do not include these backend APIs
from other subsystems. A future Vehicle Controller interface remains a team
agreement. It must arbitrate requests, expiry/cancellation and global stops.

## Hardware evidence and limits

Authoritative local source: reference_docs/hardware/Robo Pico Board Datasheet.pdf,
PDF page 5 (pins), page 7 Table 2 (limits), page 8 Table 3 (truth table).

| Resource | Left | Right |
|---|---|---|
| A input | GP8 / PWM4-A | GP10 / PWM5-A |
| B input | GP9 / PWM4-B | GP11 / PWM5-B |

Table 3 defines LOW/LOW as BRAKE, HIGH/LOW as electrical forward, LOW/HIGH as
reverse and HIGH/HIGH as COAST (both motor outputs Hi-Z). Actual vehicle direction
requires separate measured motor polarity. Do not substitute a generic H-bridge
truth table: LOW/LOW is not coast on this board.

Table 2 lists maximum PWM frequency 20 kHz, maximum motor current per channel
1 A continuous and 1.5 A peak for less than 5 s, and input supply 3.6â€“6 V.
Duty limiting does NOT enforce those current limits. No current sensor, stall
protection or physical current limit is implemented or claimed. Motor stall
current, load, supply, wiring, thermal conditions and independent power removal
must be verified in the separately authorised hardware procedure. Board motor
buttons must not interfere with that procedure.

RP2040 register/pin references: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf
sections 2.3.1.7 (SIO), 2.19 (GPIO) and 4.5 (PWM), also cross-checked with the
installed Pico SDK 1.5.1 hardware/regs/sio.h and hardware/pwm.h.

## Configuration and PWM

No ready-to-run physical calibration/default instance is supplied. The owner
must supply clock_hz, pwm_hz, max_duty, both motor polarities, zero_mode,
reversal_guard_us, explicit calibration confirmation and resource verification.
Validation is representational consistency, not evidence of electrical safety.
Configuration and backend tables are copied at init and immutable while used.
Instances/backend contexts must be zero-initialized and have stable lifetime.

The actual template configures a 12 MHz XOSC, PLL_SYS 1500 MHz divided by 6 and 2,
and an undivided 125 MHz clk_sys. The backend checks the corresponding live
clock-tree registers and rejects other configurations; it never changes clocks.

Free-running, non-inverted PWM uses integer divider 1.0:

    period = ceil(clock_hz / requested_pwm_hz)
    TOP = period - 1
    actual_hz = clock_hz / period <= requested_pwm_hz <= 20000
    duty_ceiling_counts = floor(max_duty * period)
    requested_counts = round(min(abs(effort), 1) * period)
    applied_counts = min(requested_counts, duty_ceiling_counts)

Period must be 2..65535 and the duty ceiling must be at least one count. This
allows compare=period for true 100% output without overflowing a 16-bit compare.
With the verified template clock this implementation supports requested integer
frequencies 1908..20000 Hz. Lower frequencies requiring a divider are deliberately
rejected, not silently approximated. At 20 kHz period=6250 and TOP=6249.
PWM duty is quantized; sub-half-count effort becomes the configured zero mode.
The duty ceiling rounds DOWN so quantization cannot exceed it. Its init-only
product uses precision-checked double: a binary32 value times a 16-bit period is
exact at that precision. This prevents binary32 rounding from increasing a
ceiling just below an integer count. No double operations enter the set/service
control path; double support/cost still requires full firmware verification.

The active-direction input is PWM and the other input is static LOW. During the
PWM OFF interval both inputs are LOW: brake-decay modulation. Decay/current/noise
behaviour has not been measured. No acceleration ramp or stopping distance is
invented here.

The BSP lacks divider/counter and paired-compare operations. Localized MMIO sets
those registers and writes both compares together; same-direction changes use
the PWM's double buffer and do not stop/restart the carrier. A backend transition
first preloads both SIO bits LOW, remuxes the PWM pin(s) LOW, then disables PWM.
Coast is established by atomically raising both SIO bits, preventing sequential
LOW/HIGH stop-mode switching from creating a transient drive command. PWM disable
alone freezes its output and is never treated as a guaranteed stop.

A verified template discrepancy: include/sys/sysdepend/cpu/rp2040/sysdef.h defines
GPIO_OUT as SIO_BASE+0x008, which RP2040 Table 16 identifies as GPIO_HI_IN (QSPI).
Real GPIO_OUT is +0x010, confirmed by the datasheet and SDK. The backend uses a
LOCAL +0x010 readback constant; the shared header is unchanged. The existing
GPIO_OUT_SET/CLR and GPIO_OE offsets used here were checked against the SDK.
Stop readback checks output latch/mux/enable; it is NOT proof of physical pins,
stationary wheels or zero current.

## Startup, ownership and APIs

- drv_rp2040_bind: bind a fresh backend, no hardware access.
- drv_motor_init: validate/copy configuration and callbacks, no hardware access.
- drv_motor_arm: EXPLICIT future bench-only preparation, both wheels BRAKE.
- drv_motor_set: finite signed normalized vehicle effort for one wheel.
- drv_motor_stop: explicit BRAKE or COAST; cancels pending reversal.
- drv_motor_service: nonblocking reversal progress with standstill confirmation.
- drv_motor_disarm: brake both wheels, disable requests, preserve fault/history.

No function binds/arms this backend from subsystem2_init/start, app_main or the
producer. Automatic TEST=1 uses private fake callbacks and never references the
real backend. Powered commands require a separately approved bench procedure.

One serialized owner must protect the driver AND backend across both cores.
There is no ISR-safe API or internal lock in M2. No unrelated blocking/network
work may be placed in a future motor owner. Local interrupt masking/volatile do
not provide SMP ownership. Hardware resource claims must remain exclusive.

Before claiming pins, the backend rejects enabled I2C or PTMR configuration,
non-input/foreign pin functions or overrides, live PWM slices 4/5, PWM IRQ use on
those slices, and any mirrored GPIO already assigned to PWM4/5 (GP24..27).
Physical timer numbers 5/6 map to PWM slices 4/5: reserve them with the team even
though USE_PTMR is currently zero. GP8/9 conflict with the configured I2C0 pins;
the current I2C feature is disabled. No pin/slice outside this reservation is
configured. A fresh reservation cannot detect a future concurrent claimant;
team ownership coordination remains mandatory.

The template leaves PWM reset asserted with USE_PTMR=0. The backend only clears
the PWM reset bit, never asserts a global reset, and uses at most 32 readiness
reads. This is a bounded readiness timeout, not a calibrated reversal delay.
A preflight refusal leaves foreign outputs untouched and cannot confirm a stop.

## State and reversal contract

| Situation | Software state / output |
|---|---|
| Pure init | Disabled; no hardware access |
| Explicit arm | STOPPED; LOW/LOW brake |
| Nonzero effort | RUNNING; one PWM input, other LOW |
| Opposite electrical request | REVERSING; immediate brake, pending effort |
| Guard elapsed but no standstill confirmation | Remain braked, WAIT |
| Guard elapsed and fresh standstill confirmation | Apply pending direction |
| Zero/explicit stop | Cancel pending; selected stop mode |
| Invalid active request/backend error/clock rollback | Latch fault; attempt both brakes |

STOPPED denotes a commanded non-driving state, NOT mechanical standstill.
Reversal requires BOTH the explicitly supplied guard interval and fresh external
physical-standstill confirmation for that wheel. M2 does not estimate standstill
or guess thresholds. Future encoder/controller integration must define the
measurement source, freshness, thresholds and confirmation semantics.

Direction history survives zero, brake, coast and disarm/re-arm, preventing
reversal-guard bypass. Repeated opposite requests replace pending magnitude but
do not postpone the guard. Stop cancels pending output. Pending requests have no
automatic expiry in this low-level module; the future command owner must enforce
expiry and cancel them. No elapsed-time-only automatic reversal is implemented.

NaN/Inf, invalid wheel/mode, timestamp rollback and backend failure latch a fault
and attempt BOTH wheels' brakes. b_stop_confirmed means backend stop COMMAND
confirmation only; false means stopping could not be established. Faults cannot
be cleared by re-arm/disarm. A separately reviewed, unpowered recovery procedure
is required; do not zero a live object to defeat history/fault protection.

## Test evidence (uncommitted working tree)

All 40 existing foundation groups remain unchanged. Both host profiles passed
54 groups total: 10 M1A, 30 M1Bâ€“M1D, 14 M2, zero failures.

| ID | Added coverage |
|---|---|
| S2-U43 | Configuration, frequency ceil, limits, invalid/unknown candidates |
| S2-U44 | Full/half duty, finite clamping, quantized zero, downward duty cap |
| S2-U45 | Independent motor polarity/direction selection |
| S2-U46 | Brake/coast truth-table and Â±zero behaviour |
| S2-U47 | Pure initialization, explicit preparation, copied settings |
| S2-U48 | Both reversal directions, exact guard, mandatory confirmation |
| S2-U49 | Zero/brake/coast cannot bypass direction history |
| S2-U50 | Pending effort replacement and stop cancellation |
| S2-U51 | Null APIs/callbacks, invalid wheel/stop mode, both-wheel fault stop |
| S2-U52 | NaN/Â±Inf input, latched fault, refusal to resume |
| S2-U53 | Partial prepare/apply failure and unconfirmed stop |
| S2-U54 | Timestamp rollback/wrap fault behaviour |
| S2-U55 | One wheel reversing while the other runs/stops independently |
| S2-U56 | Disarm/re-arm, preserved history and fault latch |

The fake models paired GPIO/PWM command levels, not RP2040 register timing,
transient electrical behaviour or current. Real backend register sequencing has
been statically inspected and ARM compiled, not executed under these mocks.
Actual oscilloscope and on-target checks remain mandatory. No automatic target
test initializes physical GPIO. Official testing/ records were left untouched
because they are outside the authorised directory.

Compiler: C:/msys64/ucrt64/bin/gcc.exe 14.2.0.
Common host command, from the repository root (PowerShell):

```powershell
$env:TEMP = (Resolve-Path app_program/subsystem2).Path
$env:TMP = $env:TEMP
$motorSources = @('subsystem2_test.c','motion_config.c','motion_quad.c',
    'motion_estimator.c','motion_pid.c','motion_foundation_test.c',
    'drv_motor.c','test_motor.c','drv_rp2040.c') |
    ForEach-Object { 'app_program/subsystem2/' + $_ }
$motorWarnings = @('-Wall','-Wextra','-Werror','-pedantic','-Wconversion',
    '-Wsign-conversion','-Wshadow','-Wstrict-prototypes','-Wmissing-prototypes',
    '-Wformat=2','-Wdouble-promotion','-Wundef')
& C:/msys64/ucrt64/bin/gcc.exe -std=c99 @motorWarnings `
    -DMOTION_CONFIG_HOST_TEST @motorSources `
    -o app_program/subsystem2/motor_host.exe
& ./app_program/subsystem2/motor_host.exe
# Second profile: same command plus -O2 -fshort-enums, output motor_opt.exe.
```

Final host compiles emitted no warnings; both executable exits were zero. GCC
-fanalyzer compiled drv_motor.c at -O2 without diagnostics. Production whitespace,
80-column, EOF and <=100-line function checks passed. Barr-C source ordering,
fixed-width arithmetic and unsigned bit manipulation are applied. Native float
alias, justified init-only exact double ceiling, and inherited T-Monitor/host test adapter deviations follow the accepted
foundation conventions; test formatting is not claimed as a full certification.

ARM verification: GCC 10.3.1 with professor Make-derived Cortex-M0+ GNU11 -O2
software-float/SMP flags. The unchanged Make rules were queried with -n TEST=1
for four objects: drv_motor, drv_rp2040, test_motor and subsystem2_test. Commands
were executed with only output/dependency paths redirected INSIDE Buddy 2; the
normal variant substitutes APP_TEST=0. Both TEST=0 and TEST=1 compiled all four
successfully. No shared build-generated files were changed in M2.

Supplemental strict ARM checks passed in both profiles after marking unchanged
professor include/config directories as system headers. Without that isolation,
strict checks report existing tk/typedef.h non-prototype function-pointer types
and tk/errno.h undefined TK_SUPPORT_SERCD. No professor header was modified or
production build flag changed to suppress these. Initial Buddy 2 compilation
errors (test-helper name collision and missing config_device.h include) were
corrected before final successful runs; earlier diagnostics remain in records.

Saved evidence (byte-preserved relocation during the approved folder cleanup):
[default host output](../../testing/subsystem2/logs/2026-10-14_m2_host_default.txt),
[optimized host output](../../testing/subsystem2/logs/2026-10-14_m2_host_optimized.txt),
[ARM compiler history](../../testing/subsystem2/logs/2026-10-14_m2_arm_compile.txt)
and [original Make recipe](m2_make_recipes.txt) (exact compiler invocations/results).
The [archive manifest](../../testing/subsystem2/logs/2026-10-14_evidence_manifest.md)
records original names/hashes; [official results](../../testing/subsystem2/TEST_LOG.md)
retain the uncommitted runs as preliminary evidence. Historical outcomes above
are unchanged.
Generated executables/objects/dependency files are removed after verification.
Full linking was NOT attempted again and is not claimed: the known SDK 1.5.1
versus required 2.x USB include-layout mismatch remains outside M2 scope.

## Remaining approval and hardware work

Before any powered procedure: verify physical wiring/polarity, resource
reservation, supply/load/stall current, board button state, measured PWM/duty and
startup/stop transitions. Validate brake/coast behaviour and reversal guard with
external current protection and a verified physical-standstill source. Determine
stopping distance/current and any acceleration limits by measurement. Validate
SMP serialization, execution time, reset readiness and emergency-stop priority
when RTOS integration is separately authorised. A successful compile is not
hardware, safety, current-limit or closed-loop performance validation.

No encoder GPIO interrupts, RTOS integration, motor actuation or Git mutations
were performed. A before/after inventory verifies repository changes are confined
to app_program/subsystem2/; all teammates/shared source/build files are preserved.

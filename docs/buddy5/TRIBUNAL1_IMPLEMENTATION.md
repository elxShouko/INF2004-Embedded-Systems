# Buddy 5 Tribunal 1 implementation record — 2026-10-21

This is the current representation audit and work record for branch
`buddy5-obstacle-actual`, based on `d4c91f271c9801802e3994967c96e9ec8fcba38c`
plus the uncommitted changes recorded in the test evidence. The branch started
clean and at the same commit as local `main`. No branch switch, dependency
download, commit, push, flash or hardware operation was performed.

## Requirements and repository findings

The professor's Project Write-up requires obstacle detection, ultrasonic
profiling, collision-free bypass, original-line reacquisition, prudent resource
use, C/Pico SDK and micro T-Kernel. The Project Briefing presents the five
subsystems coordinated by a Vehicle Controller. Week 6 FR10–FR14 and the Buddy 5
design retain sensing/planning ownership while Buddy 2 actuates motors; they
explicitly leave physical scan/servo parameters to calibration. Local project,
Week 6, Barr-C and relevant hardware PDFs were inspected through extracted text;
some PDF font/image content did not extract reliably. No electrical sign-off is
inferred from that inspection. The sensor specification linked below was also
checked directly.

The pasted task list is a requested audit, not evidence of functioning hardware.
The code was inspected in `app_program/subsystem5/`, `vehicle.c`, `vehicle_rt.c`,
`gpio_bank.c`, startup, Make profiles, S1's telemetry contract, and the host tests.

| Deliverable / claimed feature | Actual state and evidence | Remaining dependency |
|---|---|---|
| GP15/PWM7B servo, GP26 trigger, GP6 echo | Implemented opt-in `drv_s5`; host MMIO tests exercise real adapter code | Wiring, electrical limits, pulse mapping and target timing unverified |
| Acquisition state machine | Implemented `s5_acquire_open/step/close`; bounded nonblocking settling/echo/scan deadlines; strengthened in this change | Real edge loss, jitter and sensor accuracy need target tests |
| Forward monitor and stopped coarse/fine scan | Implemented acquisition plus policy, with forward validity/freshness and stationary evidence | Angles/orientation/settling and obstacle thresholds uncalibrated |
| Profiles and runtime policy | Implemented `s5_obstacle_logic` / `s5_obstacle_runtime`; conservative range-only candidate | Angular span and side rays do not prove physical width or corridor clearance |
| Configured bypass/recovery | Implemented eight-action configured planner and central controller; exercised by host integration fixtures | No production certificate source for free corridors/original line; no physical bypass evidence |
| Task and GPIO IRQ integration | Implemented `s5_live` core-1 owner, priority 20, static 4096-byte stack, inheritance mutex, dedicated fault flag; shared dispatcher subscription | WCET, actual stack headroom, stop latency and SMP timing unmeasured |
| Faults and cancellation | Persistent fault channel independent of telemetry; acquisition closes outputs and requires reboot | Fault-cause granularity and mechanical servo failure detection are limited |
| Compact representation and numerical analysis | This audit, private-owner compaction, explicit driver state, range/time checks and tests | Shared representation changes require coordinated review |
| Before/after evidence | Host `sizeof`, compiled ARM ABI sizes, static symbols and compiler stack estimates saved | No runtime speedup or measured target timing claim |
| Calibration and physical avoidance/recovery | NOT RUN | Hardware authorization, measurements, coverage model and repeatable test records |

Conflicts to take to the team:

- Root README and AGENTS.md inspection notes still describe absent controller
  or absent S5 integration; current code implements both. Older placeholder
  rows in OBSTACLE_SYSTEM.md are historical and superseded by its current
  integration section and this record. Shared README/TEAM/AGENTS were not edited.
- Week 6 proposes separate monitor/handling tasks; current implementation uses
  one bounded owner. This is an implementation/design difference requiring
  target timing review, not evidence that the academic goal is met.
- Week 6 leaves the servo port unconfirmed among GP12–GP15; the implemented
  backend accepts only explicitly confirmed GP15. That restriction is not a
  finding about the physical wiring.
- The pasted request mentions an agreed binary telemetry format. Current
  production telemetry is compact JSON schema v2, documented in
  `docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md`; no binary S5 protocol was found.
  Its forward distance, validity bit and source age exist; detailed S5 fault,
  profile and route fields are not transmitted by that schema.

## Representation decisions and bounds

Minimum bits below describe the numeric domain, not a packed C/wire format.
Storage remains naturally aligned. No `packed` structs, bitfields or global
`-fshort-enums` change was introduced. Public scan/profile/route/policy/live
structures and function signatures are unchanged; only S5's internal driver
and acquisition-owner layouts changed. Rebuild firmware objects together.

| Owner / fields | Existing storage; meaningful range / resolution | Minimum numeric bits; decision |
|---|---|---|
| `s5_scan_sample_t.angle_deg` | uint8, commanded 0..180 degrees; profile scan accepts 30..150 | 8; retain integer degrees |
| `.distance_cm`, acquisition/live/runtime forward distances | uint16, valid 2..400 whole cm | 9; retain 16 with independent validity; do not reserve a valid distance as timeout |
| `.captured_at_ms` | uint32, full modulo millisecond clock | 32; retain rollover domain |
| `.valid` | bool, false/true | 1 logical bit; retain bool and existing shared layout |
| Coarse/fine sample arrays | 5 / at most 9 records; 12 bytes per record on measured ABIs | 60 + 108 bytes per pair; no public reordering in this change |
| `s5_scan_context_t.started_at_ms/completed_at_ms/evaluated_at_ms` | uint32 modulo ms | 32 each; retain |
| `.max_duration_ms/max_age_ms` | uint32, validated positive <= 0x7fffffff in profile logic | 31 each; retain natural 32-bit arithmetic |
| `.max_adjacent_range_delta_cm`, acquire config `range_delta_cm` | uint16, accepted positive 1..65535 cm (actual valid range differences <=398) | 16 for existing configuration domain; retain, no new calibration threshold |
| `.acquisition_status` | enum, 0..7 | 3 logical bits; keep public enum and all fault categories |
| `.complete/cancelled/servo_settled/vehicle_stationary` | four bools | 4 logical bits; retain separate public fields |
| `s5_obstacle_profile_t.closest_distance_cm/left_min_range_cm/right_min_range_cm` | uint16, 2..400 cm when corresponding validity/sample count permits | 9; retain 16 |
| `.closest_angle_deg` | uint8, 30..150 degrees in accepted scans | 8; retain |
| `.lateral_offset_cm` | int16, conservative accepted domain -400..400; generated projection is narrower | 10 signed; retain signed 16 and positive-right convention |
| `.forward_distance_cm` | uint16, nonnegative projected whole cm up to 400; planner requires 2..400 | 9; retain; a near off-axis projection can be rejected by planner |
| `.estimated_width_cm` | uint16, current fine-span approximation 0..291 cm | 9; retain approximate label; not an enclosing physical-width bound |
| `.left_valid_samples/right_valid_samples` | uint8, each 0..2 from five coarse positions | 2 each; retain bytes |
| `.valid/bypass_direction` | bool plus enum 0..3 | 1 + 2 logical bits; retain public fields |
| `s5_obstacle_plan_t.actions[8]/action_count` | enum 0..7 per action, uint8 count 0..8 | 3 / 4 logical bits; preserve public action semantics and enum ABI |
| `.bypass_direction/reverse_candidate/executable` | enum 0..3 and two bools | 2+1+1; keep; a candidate is never executable by implication |
| `.distance_um[8]/speed_mm_s` | int32 um and signed int32 mm/s; configured route speed 1..1000 | Preserve 1 um and 1 mm/s interfaces; no scaling/precision loss |
| `.scan_id_us/valid_until_us` | uint64 full raw timer us | Retain64; identity/expiry must not alias at low 32-bit wrap |
| `s5_clear_box_t` four coordinates, `s5_clearance_t.corridor[3]` | signed int32 um, min<max, all swept bounds must fit | Keep 32; incoming certificates can span signed coordinate domain |
| Clearance scan ID/observation/expiry | three uint64 raw us | Keep 64; scan-specific observation and route-lifetime checks remain |
| Clearance verified/original-line flags | two bools | Keep; never manufactured from range-only sensing |
| `s5_route_config_t` geometry/margins/search | signed int32 um, positive 1..1000000; sensor offset 0..1000000 | At least 20 unsigned numeric bits, signed coordinate calculations wider; preserve int32 interfaces |
| Route `speed_mm_s/step_timeout_us/route_timeout_us/b_calibrated` | int32 speed 1..1000; uint32 step>0 and <route; route <=60000000; bool | 10 / up to26 /26 /1 logical bits; retain reviewed types and calibration gate |
| `s5_runtime_config_t` six timeout/age fields | uint32, positive <=0x7fffffff ms | 31; retain modulo half-range arithmetic |
| Runtime `obstacle_threshold_cm/confirmation_count` | uint16 threshold2..400; uint8 confirmations1..255 | 9 /8; retain |
| Runtime input/status timestamps and owner state-entry/last-forward timestamps | uint32 modulo ms | 32; retain; same-tick acknowledgements do not correlate a new request |
| Runtime input scan pointers/counts | const pointers, size_t, coarse 5/fine <=9; validate before copies | Preserve public pointer/count signatures and caller ownership |
| Runtime status coarse/fine counts | size_t, bounded 5/9 | Could fit bytes, deferred shared layout change |
| Runtime state/fault/scan-status/acquisition-status | enums with maxima 6/11/10/7 | 3/4/4/3 logical bits; keep public enums; target enum sizes measured rather than assumed |
| Runtime input/status validity, lifecycle and controller-owned booleans | bool fields; independent facts | Retain separate flags; no racy packed read-modify-write |
| `s5_runtime_events_t` five requests | bool levels, retryable until matching acknowledgement | Retain five bytes; these are not a queue or one-shot wire event |
| Runtime owner `obstacle_confirmations/have_update/have_forward_sample/initialised` | uint8 counter0..255 plus bools | Counter already byte-sized; no narrowing |
| `s5_acquire_config_t.sample_period_us/echo_timeout_us/servo_settle_us/scan_timeout_us` | uint32 us; sample60001..999999 and >echo, echo>=400*scale and<1000000, settle1..999999, scan>sample/settle and<10000000 | Up to20/20/20/24 bits; retain 32; do not reinterpret as ms |
| Acquire config servo period/min/max | uint16 us, 0<min<max<period<=65535 | Retain16 and 1 us precision |
| Acquire config `echo_us_per_cm/servo_gpio` | uint16 scale1..1000, uint8 GPIO exactly 15 | 10 /4 numeric bits; retain existing configuration layout |
| Acquire config wiring/calibration flags | bool | Retain explicit independent admission gates |
| `s5_acquire_io_t`, driver IRQ hooks | context/function pointers | Native ABI; no narrowing or pointer encoding |
| Acquire `index/fine_count` | size_t -> uint8, index 0..9 during transitions; fine count0 or3..9 | 4 bits sufficient; store bytes, validate bounds before array access; use local size_t for generator output then checked cast |
| Acquire `state` | enum -> uint8,0..3 | 2 bits sufficient; store byte, reserved values fault |
| Acquire seven raw timestamps/deadlines | uint64 us | Keep 64; reject regression and deadline overflow |
| Acquire `forward_cm`, angle array, five lifecycle bools | uint16 cm, uint8[9] degrees, five bools | Preserve values and flags; reorder private owner for alignment |
| Driver rise/fall timestamps and width | uint64 us / uint32 us; width checked positive <= configured echo timeout before cast | Up to20 pulse bits with current config; uint16 is insufficient; timestamps remain 64 |
| Driver armed/rise/complete/error | four bools -> volatile uint8 state 0..4 | 3 bits sufficient; byte state forbids contradictory combinations;5..255 report error |
| Driver reservation/claim flags, copied config | two bools plus existing config | Retain ownership independently of acquisition state |
| `s5_live_config_t` | acquisition+policy configs | Preserve public immutable copied layout |
| `s5_live_status_t` scan/start/completion/forward times | uint64 raw us, with source age max uint32 us | Keep 64; live max age <=999000 us and explicit validity/fault/configured booleans retained |
| Live recovery/pending IDs and motion-source timestamp | uint64, exact scan identity / source time | Keep 64; no truncation of completion correlation |
| Live kernel handles | RTOS ID / ER conventions | Preserve signed SDK/RTOS types; negative errors remain representable |

The 32-bit host default enum ABI differs from the measured ARM ABI. Neither
native `sizeof` nor a struct memory dump defines a portable telemetry encoding.
Reordering shared sample fields could reduce 12 to 8 bytes on these ABIs, but
would alter offsets and positional initializers used in tests and providers.
That remains a concrete review proposal for Buddy 5 and controller consumers,
with a full rebuild and caller/test update required before implementation.

## Pulse timing, faults and concurrency

For configured integer scale k us/cm, accepted raw widths are **2*k..400*k**
inclusive and no greater than echo timeout. At fixture k=58 this is 116..23200 us;
the configuration permits k=1000, so 400000 us must remain representable. These
are software bounds, not calibrated measurements. Conversion floors width/k
to whole cm, preserving the existing conservative range representation. Widths
just above400*k previously rounded back into 400 cm; they now fault first.
The pulse must also fit between trigger-start time and its capture timestamp.

Rise/fall comparisons and subtraction stay unsigned64. Only a validated width
<=echo_timeout<1000000 is converted to uint32; no signed conversion is used.
Raw high/low/high timer reads retry at most twice. Low 32-bit wrap is accepted;
raw 64-bit regression/overflow admission fails. Profile/policy millisecond clocks
retain uint32 wrap semantics with configured intervals below half range.

No echo faults exactly at the echo deadline. Capture after the deadline faults;
an on-time capture serviced by a delayed task can still be accepted. A capture
after the current task clock read is deferred to the next cycle. Live/policy
freshness is checked separately, so a historical range is never freshened by
copying or publishing it. Missing data never counts as a clear path.

The driver is confined to core 1, as is the subscribed ISR. IRQ-disabled copies
protect the two-word timestamps and width/state snapshot against that ISR.
Volatile marks ISR-shared storage; it does not provide atomicity. Explicit GNU
compiler barriers keep task copies/arming within DI/EI because the existing
port's inline PRIMASK restoration lacks a memory clobber. This GNU syntax is a
documented toolchain-specific Barr-C portability deviation. No kernel change,
ISR blocking, print, allocation, network operation or cross-core spinlock was
introduced. Cross-core clients use `s5_live_status`'s inheritance mutex and the
dedicated RTOS fault latch. The ISR state uses whole-byte assignments, not
shared flag-mask read-modify-write operations.

Faults stop the capture state: repeated rises, a fall without rise, nonpositive
width, over-timeout width or timer-read failure cannot overwrite a later valid
capture after error. Completed captures remain readable until the owner starts
the next trigger. The host boundary-delivery test checks copied-snapshot
semantics; it cannot prove actual interrupt masking, SMP ordering or latency.

Existing limitations: no-rise, stuck-low, disconnection and missed rise can
share the timeout diagnosis; a high ECHO at trigger admission reports trigger
failure rather than a separately measured electrical cause. Scan cancellation,
motion and timeout close the port. Servo API failure is detected, but a servo
that accepts PWM and mechanically stalls is unobservable: settling time is not
position feedback. Adjacent scan range consistency exists; the current API has
no separately calibrated forward temporal-rate filter. These are not silently
labelled solved by compaction.

## Memory and timing evidence

Identical probes before/after, all sizes in bytes:

| Type / allocation | Host normal before -> after | Host O2/short enums before -> after | Compiled ARM before -> after |
|---|---:|---:|---:|
| Acquisition owner |384 ->360|368 ->352|336 ->328|
| Driver owner |96 ->88|96 ->88|80 ->72|
| Scan sample |12 ->12|12 ->12|12 ->12|
| Scan context |32 ->32|28 ->28|28 ->28|
| Profile |24 ->24|20 ->20|20 ->20|
| Plan |96 ->96|64 ->64|64 ->64|
| Runtime input |80 ->80|80 ->80|56 ->56|
| Runtime status |392 ->392|344 ->344|336 ->336|
| Runtime owner |440 ->440|392 ->392|384 ->384|
| Live status |440 ->440|392 ->392|384 ->384|
| Acquisition config / I/O |32 /48 unchanged|32 /48 unchanged|32 /24 unchanged|
| Runtime config / events |28 /5 unchanged|28 /5 unchanged|28 /5 unchanged|
| Clear box / clearance / route config / live config |16 /80 /56 /60 unchanged|same|same|

Compiled ARM named S5 static-symbol sum: **5399 ->5383 bytes**, including the
unchanged 4096-byte stack. This 16-byte saving is the sum of symbol sizes, not a
linked total-RAM/high-water measurement; linker alignment and kernel object
allocation remain outside that sum. No queue capacity or message size changed.
S5 still owns one task, mutex and flag; no new heap allocation was introduced.

ARM GCC -fstack-usage estimates: acquisition step 88 ->88, driver ISR 20 ->20,
echo snapshot 16 ->16, trigger 24 ->16, live task 728 ->728 bytes. These are
individual compiler frame estimates, not call-chain maxima or stack headroom.
There is **no measured execution-time or WCET comparison**. Compiler-generated
code sizes and full frame listings are saved in the size logs; smaller memory
or object code is not evidence of faster/safe physical control.

Before: `testing/subsystem5/logs/buddy5_20261021_sizes_before_fixed.txt`.
After: `testing/subsystem5/logs/buddy5_20261021_sizes_after.txt`.
Two earlier probe attempts failed due missing host-fake preprocessor flags;
their `sizes_before` / `sizes_baseline` logs are retained as harness failures,
not measurements or firmware regressions. The corrected baseline was measured
before production changes.

## Integration and feasibility decision

The existing controller polls a dedicated fault latch ahead of ordinary
providers. S5 requests/publishes through the controller; it never writes drive
motor GPIO. Stopped scanning needs fresh S2 measurement validity, standstill,
READY and no active command, plus the policy's post-request acknowledgement.
Controller finite moves retain command admission/completion IDs and terminal
acknowledgement; S5 recovery uses the exact 64-bit stopped-scan token.

The configured route requires calibrated geometry and three verified free
corridors covering rotation, translation, return/search, braking and tracking
error. Evidence must match the scan and outlast the bounded route. The
range-only planner remains CLEARANCE_UNVERIFIED with zero executable actions;
the live controller holds a valid scan rather than inventing a certificate.
The search requires original-line identity evidence; a LINE_FOUND alone does
not establish that identity. S3 mode transitions stay controller-owned.
Profile projections use approximate fixed-point trigonometry and whole-cm
rounding. They are not guaranteed enclosing obstacle coordinates. A reviewed
geometry/error model must bound range quantization, projection error, beam width,
mounting error and motion before any clearance certificate is meaningful;
none of those margins has been physically established here.

**Current decision: autonomous physical bypass is not established.** A single
side ray and approximate span cannot validate blind/rear/swept sectors, moving
obstacles or original-route uniqueness. No current default producer supplies
the required evidence. Calibration alone cannot fix missing observability.
An invalid/malformed input is rejected; a plausible range-only route remains
unverified; executable means accepted under supplied certificate/configuration
assumptions, not independently proven safe by this sensor. A new route-quality
enum or telemetry/fault schema would be a shared interface change requiring
team review; none was invented here.

## Electrical and servo configuration record

| Item | Code / documentary constraint | Measured value / status |
|---|---|---|
| Sensor model / revision | Existing notes identify HC-SR04; verify actual variant | Unknown revision |
| Sensor VCC / echo-high | HC-SR04 reference specifies 5 V supply; ECHO compatibility must be checked before connection | Existing notes report 3V3 VCC; not electrically verified |
| ECHO level shifting / common ground | RP2040 GPIO compatibility must be established; do not assume a 5 V ECHO is compatible | Circuit, ECHO high/low and common ground unmeasured |
| Trigger / echo | GP26 SIO output / GP6 input through shared core-1 dispatcher | Physical wiring unverified |
| GP26 PWM alias | PWM5A aliases GP10 motor channel; trigger remains SIO | Code inspected; no target measurement |
| Servo signal | Explicit GP15/PWM7B only; rejects active slice 7 / GP14 PWM alias | Trace connector-to-pin before confirmation |
| Servo model / supply / current | No model or confirmed power arrangement supplied | Unknown; measure supply/peak demand and ground arrangement |
| Servo period / min / max / orientation | Caller-supplied us endpoints; fixed 0..180 logical mapping, 90 forward, smaller left | All physical mapping values uncalibrated |
| Clock | Driver checks template 125 MHz before PWM divider 125 for 1 MHz counter | Not measured on target |
| Settling / motion feedback | Configured nonblocking delay; no position feedback | Stall/actual angle cannot be certified by software delay |
| Trigger/cycle | >=10 us trigger; configured measurement interval >60000 us | IRQ-enabled 10 us BSP pulse can stretch under preemption; scope evidence absent |
| Distances, filtering, vehicle geometry and stopping | Mandatory configuration/certificates; no production defaults supplied | Unmeasured |

Sensor specification: [HC-SR04 datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf).
Local board sources: `reference_docs/hardware/Robo Pico Board Datasheet.pdf` and
`RP-008312-DS-2-pico-w-datasheet.pdf`. No electrical or hardware-test authorization
was inferred. Keep wiring/calibration flags false until measured and reviewed.

The coarse 30/60/90/120/150 and fine 5-degree geometry remain existing development
assumptions; making the shared profile geometry configurable requires coordinated
generator/validator/caller changes. The Servo endpoints/settling and range scale
are configurable already. None was relabelled calibrated.

## Reproduction and remaining tests

Use the installed Python executable (the WindowsApps `python` alias is broken
on this machine):

```powershell
$pythonExe = 'C:/Users/User/AppData/Local/Programs/Python/Python314/python.exe'
& $pythonExe testing/subsystem5/measure_representation.py --label UNIQUE_sizes
& $pythonExe testing/subsystem5/verify_representation.py --label UNIQUE_bounds
& $pythonExe testing/integration/verify_integration.py --label UNIQUE_host
& $pythonExe testing/integration/verify_integration.py --label UNIQUE_arm --arm
```

Labels must be new because evidence files are never overwritten. The scripts
use existing ignored `testing/**/build` scratch folders, installed host/ARM GCC
paths and existing project flags; no new integrated build system was added.

Results: before/after host suites pass in both normal and O2/short-enum profiles;
79 integration IDs and all 113 Buddy 5 IDs S5-U02..U114 pass per final profile
(the 113 comprise 107 from the integration runner plus 6 numerical cases).
Eight existing integration mutations and two new acquisition mutations are
detected per profile. All 44 final ARM objects compile across SMP 0/1 and TEST 0/1.
On-target S5-U01 was not executed.

`make -j8` and `make TEST=1 -j8`, default SMP 1/usb_cdc/cyw43/NET=none/MQTT 0/
CRASH_REPORT 0, were attempted with installed MSYS GNU Make as a Windows fallback.
Initially PICO_SDK_PATH was unset. Retrying with the installed SDK 1.5.1 fails in
the unchanged USB compatibility sources on missing `pico.h`; no firmware image
was linked. README documents SDK 2.2.0; no local `build/deps/pico-sdk` exists here.
WSL lists only docker-desktop, not Ubuntu. The failure logs are retained; no SDK
was downloaded and no Makefile was patched to conceal the prerequisite issue.

Remaining work: authorized SDK 2.2.0/WSL build setup and full normal/TEST linking;
electrical verification; measured servo/range/geometry/stopping configuration;
target IRQ/jitter/stop/stack tests under encoder and network load; verified free
space/original-line evidence source; and physical bypass/recovery trials with
video evidence through `videos/README.md`. Future manual cases S5-U115..U119
are defined in the subsystem test log with no claimed result.

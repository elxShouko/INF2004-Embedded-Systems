# Buddy 2 and shared data representation review

## Current Buddy 2 increment - 2026-10-28

The current implementation is based on merged HEAD
93740736dd1c3407eaf7f496089a096cee70c140, not the October 21 candidate.
The historical proposal below is retained, not applied wholesale. Current
S1 binary telemetry/map gateway, S3 packed queues and barcode-then-junction
behaviour, S4 packed IMU status and S5 echo ownership are preserved.

Implemented only inside Buddy 2:

- drv_motor_t's five scalar owner flags, motion_quad_t's three capture flags
  and wheel_loop_t's three scalar flags use one-bit bool fields.
- motion_bus_config_t stores configuration and callback port together;
  motion_bus_payload_t overlays CONFIG, COMMAND and ACK payloads. ENABLE has
  no payload. The full-width request tag is validated before variant reads.
- All internal producers/consumers select payload.configure, payload.command
  or payload.ack_id. Existing task mutex, copied lifetime and emergency flag
  paths remain. No pointer aliases caller-owned command storage.
- S2-U141..U143 exercise flag independence/copy, selected payload copy and
  invalid-tag rejection. Existing lifecycle and mutation checks are retained.

Public motion API signatures, numerical types, measurement units/timestamps,
loop periods, PID calculations, GPIO, task startup and priorities are unchanged.
No teammate implementation or shared production header is edited. The request
type is internal but transitively visible; rebuild affected consumers together.

### Matched ARM structure measurements

| Type | Merged before bytes | After bytes |
|---|---:|---:|
| drv_motor_t | 112 | 104 |
| motion_quad_t | 24 | 16 |
| enc_capture_t | 56 | 40 |
| wheel_loop_t | 752 | 736 |
| motion_run_t | 984 | 968 |
| motion_bus_request_t | 256 | 216 |
| motion_bus_t | 400 | 360 |
| motion_run_status_t | 136 | 136 |
| move_plan_command_t | 40 | 40 |
| vehicle_input_t | 400 | 400 |

Before and after layout.json/inventory.md are under
testing/subsystem2/logs/minbits_20261028_layout_before and
minbits_20261028_layout_after. Current probe supports flat typedef unions and
bit-fields. C forbids individual sizeof/offsetof for bit-fields; those offsets
and byte counts are recorded as null, not invented. Width is the declaration's
bit count; whole structure sizes are measured with the target compiler.
The before inventory has 35 types/299 fields; the after has 37/301 because the
configuration pair and union are now separate types. Union members overlap;
do not sum their sizes or nested structure savings as total RAM.

### Remaining compliance decisions

This increment is partial, not all-application-data compliance. Public enums,
status flags, indexed bool arrays, calibration, IDs, diagnostics and numerical
storage still need range/representation review. Widened validation ingress is
retained intentionally; truncation before validation is unacceptable.

Counts remain signed int64; speed int32 mm/s; position int64 um; finite travel
int32 um; kernel timestamps uint64 us; lifetime/IDs uint32. No smaller physical
operating envelope is asserted. Define maximum count rate and signed excursion,
uptime, command rate, scale products, rounding and overflow policy before
narrowing. PID retains its explicit binary32/nonfinite contract until numerical
error and arithmetic requirements are justified. Current saturation/fault tests
are correctness evidence, not car calibration.

S1's signed-int32 wire counts and bounded speed/ages do not define these
internal ranges. Out-of-range telemetry must remain invalid, never clamped.
S3 uses controller-adapted cumulative forward um and raw-domain time; duplicate
observations and successful originating-event completion remain controller
responsibilities. No direct S3 calls or new S1 dependency enter the motor owner.

Fault reporting is deferred pending cause translation: no-progress is not
proven motor stall; generic sensor/output/time faults must not be guessed into
the shared bits. Current urgent stopping and restart policy are retained.

The concrete shared vehicle.h flag proposal requires review against this
merged baseline before application. It must retain mapping gateway, event IDs,
packed-provider semantics and existing safety ownership. No new calibration,
emergency reset, numerical limit or mission behaviour is included here.

### Evidence and limitations

All six matched firmware links pass. Static BSS decreases by 72 bytes in each
profile; initialized data is unchanged. Non-TEST text decreases by 12 bytes;
TEST text increases by 996 bytes, including the three new regression groups.
These section differences are measured whole-image results, not sums of nested
structure sizes. Host motion checks pass 141 groups per profile; integration
passes 83 unique INT groups including the separate TEST startup check. Strict
ARM checks compile 76 Buddy 2 and 44 integration/provider objects. S1 and S3
owner-maintained host suites also pass. Hash comparison confirms 78 peer files
and 631 shared/platform files match the captured pre-edit baseline.

Commands/results are appended to testing/subsystem2/TEST_LOG.md. Host GCC
14.2 and ARM GCC 10.3.1 use existing tools. Six matched firmware profiles use
the same local SDK 2.2.0 and published example credential fixture. This is not
the October 26 SDK 2.3.1 artifact set; compare only these matched builds.
Other owners' pre-existing conversion warnings stay visible and unchanged.
No on-target execution, stack/heap high-water, WCET, ISR/stop latency or physical
accuracy is established. No hardware, installation, fetch, stage/commit/push.

## Historical scope and baseline - 2026-10-21

This is a measured before-change baseline and a proposal for review, not an
implementation or compliance declaration. The user authorised baseline tests,
field inventory and proposed layouts before review of shared interface changes.
Production sources remain at d4c91f271c9801802e3994967c96e9ec8fcba38c.
Branch: buddy2-Motor-and-encoder-control. No hardware was accessed or flashed.

The professor's Change Requirement 1 requires justified minimum-bit application
data, binary MQTT application payloads, safe packed sharing and before/after
RAM, message and correctness/performance evidence. This does not establish
physical calibration or authorise guessed bounds. The Week 6 mission remains.

Existing AGENTS.md and portions of MOTION_SYSTEM.md describe superseded
integration gaps. Current vehicle_rt.c and TEAM.md section 7 establish the
controller and dispatcher implementation. Ownership restrictions still apply.

## Measured evidence

All paths below are repository-relative. Commands, hashes and raw results are
retained; initial failures are not overwritten.

| Evidence | Result | Location |
|---|---|---|
| Buddy 2 host, strict warnings/analyser | 95 pure + 43 adapter groups per default/optimized profile; 8 mutations detected | testing/subsystem2/logs/verify_minbits_before_20261021_* |
| Integration host | 78 normal groups/profile, separate APP_TEST startup; 8 mutations/profile; S5 policy/acquisition/MMIO suites | testing/integration/logs/minbits_before_20261021_* |
| Buddy 2 ARM | 19 objects x SMP0/1 x TEST0/1 = 76 | testing/subsystem2/logs/verify_minbits_before_20261021_arm_retry_* |
| Shared/provider ARM | 11 objects x SMP0/1 x TEST0/1 = 44 | testing/integration/logs/minbits_before_20261021_arm_retry_* |
| Target declaration inventory | 35 structures and 299 member declarations; ARM size, member size and offset | testing/subsystem2/logs/minbits_layout_before_20261021_retry/layout.json and inventory.md |
| Isolated candidate layouts | Compilation/size measurement only, no behavioural tests | same directory, candidate_probe.c.txt, candidate_sizes.json, candidate_commands.log |
| Full firmware rebuilds | See per-profile results and raw Make logs | testing/subsystem2/logs/minbits_firmware_before_20261021/ |

Compiler: ARM GNU 10.3.1; host GCC version is retained in runner logs. ARM
compiler launch initially failed inside the restricted environment with Windows
error 623. Approved execution outside that restriction passed. WSL listed only
docker-desktop; existing MSYS2 Make/bash and the approved local SDK were used
for full builds, as in earlier repository evidence. No installation/download.
The first layout probe omitted subsystem2.h and failed on SAMPLE_MSG; the
corrected retry succeeded. Both probe logs are retained.

The full builds use the byte-identical published example credential fixture,
not a live network configuration. Old existing ELF sections/hashes were
captured separately in layout.json and are explicitly archival. Fresh builds
differ from those old artifacts; only matched fresh builds may be used as the
new before/after comparison. No source change is inferred from artifact age.

The inventory probe uses the archived recipe's GNU11/O2, SMP=1, APP_TEST=1
options. The inventory tool compiles array symbols with sizeof/offsetof and reads ARM
symbol lengths; it does not run target code or infer ARM layout from host ABI.
Its enum/struct parser covers the current simple typedef declarations only;
it is not a C parser, a complete firmware field audit or a proof of ranges.
Function-local values, globals, macros, unions from other owners, RTOS object
overheads and build-profile-dependent definitions need separate review.

## Range and ownership decisions

The generated inventory is the declaration-level table. This table supplies
semantic rules for field families; numeric entries remain unresolved rather
than being labelled calibrated or minimum-width compliant.

| Family / examples | Current representation / units | Required range analysis and action |
|---|---|---|
| motion lifecycle state | 7 named values | 3 logical bits; retain invalid input rejection before compact storage |
| motion result | 11 named values | 4 logical bits; terminal and independent fault result remain distinct |
| command kind | 6 named values | 3 logical bits; no reserved value becomes a valid move |
| motion bus kind | 4 named values | 2 logical bits; dispatch validation precedes variant access |
| motor state / stop mode | 3 / 2 named values | 2 / 1 logical bits; preserve reversal and brake/coast semantics |
| calibration / polarity | 3 / 3 named values | 2 bits each; zero remains unknown/unconfigured; reject unsupported values |
| Boolean members | false/true | 1 logical bit each; pack only within existing owner/lock domain |
| owner globals ready/open flags | bool | inventory separately; do not combine independently accessed lifecycle/safety objects |
| quadrature previous_ab | uint8, AB in 0..3 | 2 bits after validation; previous-state invalid-value tests must remain meaningful |
| decoder direction | int8, -1/0/+1 | 2-bit code possible with explicit mapping; do not rely on signed bit-field implementation |
| encoder count | signed int64 x4 transitions | maximum signed excursion from boot origin, rate, duration; no live reset, no wrap; preserve fault at limit |
| encoder generation | uint32 | nonzero generation and reset semantics; no fabricated reboot identity |
| invalid transition diagnostics | uint32 saturating | recurrence/count reporting requirement before narrowing |
| wheel position / origins / remainder | int64 micrometres or numerator remainder | scale products, travel lifetime, negative motion and fractional carry; intermediate and stored bounds separately |
| measured/target wheel speed | int32 mm/s | physical envelope plus accepted software range; preserve reverse and low-speed resolution |
| travel/turn command targets | int32 micrometres | maximum per-command magnitude, tolerance, turn scaling and checked products |
| slew velocity and remainders | int64 um/s, uint64 remainder | elapsed-time and acceleration products; preserve fractional accumulation |
| PID gains/integral/effort | guarded binary32 | precision and finite range per quantity, not an automatic exemption; clarification and error analysis required |
| geometry / encoder scale | uint32 counts/rev, um | physical calibration, valid software products and zero-invalid semantics |
| PWM period/duty/pending level | uint16 | actual clock/frequency range and 100% endpoints; MMIO callback types separately |
| clock / PWM frequency | uint32 Hz | supported 125 MHz backend, accepted PWM range, discrete encoding proposal only after review |
| operating timestamps | uint64 kernel us | kernel epoch, resolution and supported uptime; preserve monotonic/error checks |
| raw source timestamps | uint64 raw us or source modulo ms | do not equate raw/kernel epochs; define wrap horizon before reduction |
| periods/ages/deadlines | uint32 us/ms | tick quantisation, maximum interval, stale/future/overflow; never round old data younger |
| request and terminal IDs | uint32 | nonzero monotonic IDs, same-ID refresh, no wrap; define exhaustion before narrowing |
| vehicle blockers | 5 independent bits in uint32 | 5-bit report possible; local safety latches stay separate |
| vehicle state | 11 named values | 4 bits; validate enum before compact assignment |
| vehicle booleans | false/true | owner context versus mutex-published copy; no new multiwriter fault word |
| odometry half_um | uint8, carry 0/1 | 1 bit with invariant; relative forward distance and validity remain separate |
| route index | uint8 | action-count bound plus terminal sentinel; coordinate with S5 owner |
| GPIO masks/MMIO callbacks | uint32/pointers | hardware bit positions/ABI, not sequential enum values; do not truncate a GP28 mask to 5 bits |
| legacy SAMPLE_MSG | T_MSG + UW/INT values | retained S3/S4 demo dependency; coordinated retirement/representation review required |
| copied S1/S3/S4/S5 provider members | owner-defined nested types | inventory by their owners; preserve source meaning in controller; no edits to buddy implementations |

The current motion_bus request is one copied slot under the existing
inheritance mutex, not a dynamically allocated queue. Its config, command and
ack data are mutually exclusive by kind, but existing storage reserves all.

## Isolated candidate measurements

These are target-compiled declarations, not changes applied to production.
Boolean candidates change only scalar bool members of the named structure to
one-bit _Bool fields, leaving nested types unchanged. They are not cumulative
whole-system predictions, portable wire formats or timing/safety evidence.

| Structure | Before bytes | Isolated candidate bytes | Observation |
|---|---:|---:|---|
| drv_motor_t | 112 | 104 | five scalar owner flags share bits |
| motion_quad_t | 24 | 16 | three scalar owner flags share bits |
| wheel_loop_t | 752 | 744 | scalar flags only; bool arrays remain unchanged |
| motion_run_status_t | 136 | 136 | flag packing alone saves no structure bytes |
| motion_bus_t | 400 | 400 | flag packing alone saves no structure bytes |
| vehicle_config_t | 96 | 92 | scalar flags packed; shared setup rebuild required |
| vehicle_input_t | 416 | 408 | scalar flags packed; nested provider types untouched |
| vehicle_t | 288 | 288 | tail/alignment offsets consume the apparent saving |
| motion_bus_request_t | 256 | 216 | named tagged union; full original numerical widths |

### Proposed Buddy 2 first patch

1. Pack scalar owner-only bool members in drv_motor_t, motion_quad_t and
   wheel_loop_t with bool one-bit fields; preserve existing lock/owner rules.
   Review every access/address use and generated loads/stores first.
2. Replace internal motion_bus_request_t payload with a named union:

```c
/* Candidate shape, not a committed interface. */
typedef struct
{
    motion_bus_kind_t kind;
    union
    {
        struct
        {
            wheel_loop_config_t config;
            drv_encoder_port_t encoder_port;
        } configure;
        move_plan_command_t command;
        uint32_t ack_id;
    } payload;
} motion_bus_request_t;
```

   Update task_motion.c, motion_bus.c and their tests together. Validate kind
   before reading a variant, zero-init producers and preserve a complete copied
   config/port. Never retain pointers to caller command storage. Enable has no
   payload. This is an internal layout change but transitive headers are rebuilt.
3. Do not narrow encoder counts, PID values, IDs or timestamps in this patch.
   Record them as open range/precision decisions; do not claim full compliance.
4. Preserve all current negative enum tests. A narrow public enum bit-field can
   truncate an invalid value before validation; do not replace public enums
   without a checked boundary or a reviewed equivalent rejection policy.

Changes in this proposal affect Buddy 2 representation/transport, not motor
algorithms, calibration, GPIO setup, timing configuration or safety ownership.
Scalar bool fields already normalise assignments; packing preserves those
values, but actual compiler layout and accesses require target checks.

### Concrete shared proposal awaiting review

In app_program/vehicle.h, pack only scalar bool members in vehicle_config_t and
vehicle_input_t, preserving names/semantics, controller ownership and copied
snapshot handling. This changes ABI/layout (measured 96->92 and 416->408) but
not function signatures. vehicle_setup_t and vehicle_t contain nested config;
their final sizes must be remeasured after actual nesting. Rebuild all affected
consumers together. Review any address-taking, offsetof or persistence use.

Affected owners: shared controller, Buddy 2 setup, and consumers rebuilding
transitive headers. Do not modify S1/S3/S4/S5 production or their public types.
No app_main.c/app.h, TEAM.md, Makefile, kernel, driver ownership or task-priority
change is proposed. Do not pack GPIO callbacks or touch the IRQ protocol merely
to obtain a size reduction. MQTT binary migration remains Buddy 1's work.

Risk: C bit-fields are ABI-dependent and cannot be addressed individually.
They are used only for internal C objects, never raw serialised. Packing is
not synchronisation; existing single-owner and mutex boundaries must hold.
No local interrupt-only lock is introduced for cross-core data.

## Verification for the first patch

An exact eight-file draft diff is saved at
testing/subsystem2/logs/minbits_layout_before_20261021_retry/proposed_first_patch.diff.txt.
It is applied only to an ignored isolated source tree under
testing/subsystem2/build/minbits_candidate_review, never to production.
The candidate has no new numerical bounds or changed task/IRQ ownership.
Existing Buddy 2 host tests/mutations and 76 strict ARM objects pass for this
isolated draft; 44 shared/provider ARM objects also pass. Integration host
passes 79 unique IDs/profile including separate TEST startup, eight mutations
per profile and the existing S5 policy/acquisition/MMIO suites. Initial candidate
ARM attempts lacked copied device headers; those failed logs are retained and
completed-header retries pass. This is not a full candidate firmware link,
new flag-isolation test, target run or approval to apply the draft.

Complete nested candidate ARM sizes (review_patch_sizes.json):

| Type | Baseline bytes | Draft bytes |
|---|---:|---:|
| motion_bus_request_t | 256 | 216 |
| motion_bus_t | 400 | 360 |
| enc_capture_t | 56 | 40 |
| wheel_loop_t | 752 | 736 |
| motion_run_t | 984 | 968 |
| vehicle_config_t | 96 | 92 |
| vehicle_input_t | 416 | 408 |
| vehicle_t | 288 | 280 |
| vehicle_setup_t | 432 | 428 |

Do not sum nested sizes to claim total RAM savings. Objects can contain other
listed objects, and linker allocation/stack behaviour need separate measurement.
Five draft lines were wrapped after behavioural verification. An automated
comparison confirms only whitespace changed; candidate_formatting.json records
the tested and formatted hashes. The final formatted draft passes the existing
19-file style subset. Raw tests/ARM metadata bind the pre-formatting draft;
candidate_source_hashes.json binds the review diff. No behavioural retest is
claimed for a different token stream.

Keep existing S2-U03..U140 and INT-03..INT-81, including their mutations.
New planned groups should cover:

- Independent setting/clearing and struct-copy of packed flags.
- CONFIG/ENABLE/COMMAND/ACK variant isolation and invalid-kind rejection.
- Pending/inflight/cancel/stop and durable terminal regression.
- Existing invalid enum, count-width, timestamp and emergency mutations.
- ARM sizeof/alignment and generated-access review; measure real nesting.
- Default and O2/short-enum host profiles, strict ARM SMP0/1 TEST0/1 objects.
- Matched six firmware profiles and actual static section differences.

Do not use sizeof/offsetof on bit-field members in the measurement tool after
migration; adapt the tool explicitly. A smaller message is not a smaller
allocated kernel buffer unless that allocation changes. The one-slot motion
bus has no kernel message-buffer allocation to claim. Dynamic heap/stack
high-water, WCET, ISR load and physical motor behaviour require target trials.

## Remaining decisions and owner inputs

| Decision | Owner/input | Blocks |
|---|---|---|
| Arithmetic/floating-point/mandatory ABI interpretation | professor/team | final compliance claims, not baseline tests |
| Max uptime/rate/count excursion and numeric resolution | Buddy 2/team + calibration | numerical narrowing |
| Shared scalar-flag ABI change | user/shared review | application of vehicle.h proposal |
| MQTT binary schema and receiver | Buddy 1 + all field owners | production telemetry migration |
| Provider field contracts | Buddies 3/4/5 | their message changes and final controller conversion |
| Clearance/original-route evidence | Buddy 5/shared | autonomous bypass activation |
| Wiring, power and measured operating envelope | hardware owners | target activation/calibration |

This first patch is an incremental improvement. It does not complete the
professor's all-application-data requirement or the original robot mission.

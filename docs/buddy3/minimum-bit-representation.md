# Subsystem 3 minimum bit representation

> Follow-up safety review: see [feedback audit and current contract](feedback-audit-and-contract.md)
> for checked delay failure, identifier exhaustion, odometry lifetime and current
> evidence. Earlier size/stack/flash figures below identify the packing release;
> the appended feedback run in TEST_LOG records the later firmware comparison.

The 2026-10-21 change packs S3 application state and its actual inter-task
messages while preserving the current controller's configuration, odometry,
mode, status and manoeuvre-completion calls. It does not establish compliance
for the entire vehicle: Buddy 1 still publishes JSON and the other owners'
data representations have not been changed.

Baseline: `d4c91f271c9801802e3994967c96e9ec8fcba38c`. Changes are uncommitted.
The original Change Requirement 1 document is local coursework evidence,
not a file added to this repository. It requires minimum-bit application data,
safe shared updates, binary MQTT and measured Week 13 evidence.

## Changes and compatibility

| Files | Change |
|---|---|
| `member3_data.h`, `member3_navigation.h` | Separate pure application types from kernel declarations; pack producer-owned status/output fields; replace text mapping request with `nav_update_t` |
| `drv_s3_codec.c/.h` | Explicit bit encoding, kind-specific lengths, canonical padding and enum/range validation; reject partial/malformed frames without publishing partial data |
| `member3_navigation.c` | Encode/decode inside existing APIs; allocate smaller message buffers; pack owner flags; retain mutex-protected copied status and bounded input work |
| `drv_navigation_map.c/.h` | Three bits per action, checked accessors, binary partial transactions and a checked text-boundary helper |
| `drv_barcode.c/.h` | Five-bit run count, one-bit capture flags and 234-bit Code 39 lookup table; initialise decoder scratch arrays explicitly |
| `drv_ir_input.c/.h` | Three-bit validated filter patterns and one-bit flags; validate raw input before assigning narrowed storage |
| `drv_junction.c/.h` | Three-bit phase and one-bit owner flag; preserve crossing calculations and timestamp/distance units |
| `testing/subsystem3/` | Current queue-aware fixture, codec/map regressions, ABI/allocation measurement runner and appended evidence |

No production source outside `app_program/subsystem3/` changed. The measurement
runner checks all 89 other application C/header files against the baseline.
The standalone algorithms in `src/line/` and `include/line/` remain independent;
their diagnostic keeps its original data layout.

All application source must be rebuilt together. C bit-fields are used only
for in-process storage; their compiler layout is never sent through the queue
or used as a network protocol. The queue codec defines each byte explicitly.
No global compiler packing or short-enum option is introduced.

## Data representation table

The bit counts below describe encoded application information. Byte containers,
C alignment and temporary arithmetic operands may be wider. The target ABI
measurements, rather than host enum assumptions, determine actual RAM usage.

| Application field | Valid range | Required bits | Implementation |
|---|---|---:|---|
| Input kind | 5 kinds | 3 | Queue tag; tag 7 explicitly requests a negative validation reply; 5/6 rejected |
| Output kind | 3 kinds | 2 | Output bit-field and queue tag |
| Mode | 4 modes | 2 | Status bit-field and mode-message payload |
| Line observation | 4 observations | 2 | Output/status bit-field and line-message payload |
| Navigation event kind | 7 kinds | 3 | Queue payload; native `nav_event_t` remains the controller's validation carrier |
| Navigation state | 6 states | 3 | Status bit-field and navigation queue payload |
| Navigation command | 5 commands | 3 | Queue payload and each action-map entry |
| Decoded letter | A-Z or absent | 5 | Queue codes 0-25 and 26=absent; native API retains printable character |
| Raw/normalised sensor masks | 3 digital channels, 0-7 | 3 each | Navigation queue mask and status normalised bit-field; raw ingress stays byte-wide for invalid-input detection |
| Calibration polarity and validity masks | 0-7 each | 3 each | Configuration queue fields; native validation carriers remain bytes |
| Sample period | Accepted 10-100 ms multiples of current 10 ms tick | 7 as value in ms | Queue field; 127 marks unrepresentable invalid ingress; owner retains original tick/range validation |
| Junction enabled, odometry tracking-forward | False/true | 1 each | Typed bit-fields and configuration/odometry queue bits |
| Calibrated, filtered-line-valid, accepted | False/true | 1 each | Status/reply bit-fields and reply acceptance bit |
| Sample-gap, recovery-timeout, output-full, driver, kernel fault | Absent/present per condition | 1 each | Five-bit fault mask, values 1/2/4/8/16; NONE=0 |
| Barcode run count | 0-29 | 5 | Capture bit-field; bounds checked before increment |
| Capture started/black/armed/capturing | False/true | 1 each | Owner bit-fields |
| Filter candidate/stable | 0-7 | 3 each | Owner bit-fields after raw-sample validation |
| Filter started/valid | False/true | 1 each | Owner bit-fields |
| Junction phase | 5 phases | 3 | Owner bit-field |
| Junction started | False/true | 1 | Owner bit-field |
| Owner absent/finding/recovering/previous-sample/have-odometry/odometry-forward/have-barcode | False/true | 1 each | Owner-only bit-fields |
| Action map and published map | 26 entries, 0-4 each | 78 total | Ten bytes, checked three-bit accessors; final two bits zero |
| Partial-update selection | Any nonempty subset of 26 letters | 26 | Queue selection mask; native `uint32_t` permits checking illegal upper bits before narrowing |
| Code 39 letter pattern table | 26 nine-element patterns | 234 total | Thirty ROM bytes, explicit nine-bit extraction; final six bits zero |
| Request/event/pending IDs, map revision, output-drop count | Existing uint32 domain | 32 each | Retained exactly; no new wrap or saturation semantics introduced |
| Sample/navigation/odometry times and owner time markers | Existing raw uint64 microseconds | 64 each | Retained exactly, including full source clock domain |
| Forward distance and junction distance markers | Existing uint64 micrometres | 64 each | Retained exactly; cumulative forward distance is not wheel absolute path |
| Line stable/lost/found, recovery timeout, barcode quiet/timeout/rearm, sample gap, odometry stale durations | Existing uint32 microseconds | 32 each | No new calibrated maxima invented; relationships validated by existing algorithms |
| Junction maximum bar, minimum gap/span, maximum span, clear span, maximum sample step | Existing uint32 micrometres | 32 each | Existing measured configuration contract retained |
| Junction maximum sample gap, minimum black duration, maximum crossing duration | Existing uint32 microseconds | 32 each | Existing duration contract retained |
| Captured widths[29] | Accepted 1-UINT32_MAX | 32 each | Exact widths retained; 64-bit ratio arithmetic avoids overflow |
| Calibration samples and high_count[3] | 0-UINT32_MAX, counts <= samples | 32 each | Existing overflow/range checks retained |

Transient arithmetic and decoder sorting/reversal scratch retain native fixed
widths. Public enum arguments and native `nav_event_t` are validation/API
carriers, not packed C wire structures. Fully eliminating those wider carriers
would require coordinated controller API changes. They are documented residual
coverage, not a claim that every application object is minimally allocated.

The legacy S2-producer/S3-statistics/S4-monitor data is also preserved. Its
representation needs a separately coordinated change; S3 does not silently
change another owner's example communication format.

## Queue format and measured sizes

Fields are appended least-significant bit first within each field, beginning
at byte zero bit zero. Unused final bits must be zero. No C padding crosses
the queue. The codec header defines buffer capacities; callers must supply
buffers of those capacities when using the pure codec directly.

| Input | Fields after the three-bit tag and 32-bit request ID | Bytes before on ARM | Bytes after |
|---|---|---:|---:|
| Configuration | Calibration 6, period 7, nine durations x32, junction-enabled 1, nine junction settings x32 | 392 | 79 |
| Mode | Mode 2 | 392 | 5 |
| Mapping | Selection 26, actions 26x3 | 392 | 18 |
| Odometry | Timestamp 64, distance 64, tracking-forward 1 | 392 | 21 |
| Manoeuvre completion | Event ID 32 | 392 | 9 |

Every output starts with kind 2, event ID 32, map revision 32 and faults 5.

| Output | Remaining fields | Bytes before on ARM | Bytes after |
|---|---|---:|---:|
| Navigation | Timestamp 64, black mask 3, event kind 3, command 3, state 3, letter 5 | 40 | 19 |
| Line change | Timestamp 64, black mask 3, observation 2 | 40 | 18 |
| Validation reply | Request ID 32, accepted 1 | 40 | 13 |

Configured capacity remains at least four maximum-size inputs and sixteen
maximum-size outputs. The real kernel uses a four-byte message header and
rounds each payload to four bytes:

| Target measurement | Before | After |
|---|---:|---:|
| Input buffer requested from kernel | 1,584 B | 336 B |
| Output buffer requested from kernel | 704 B | 384 B |
| Combined requested queue storage | 2,288 B | 720 B |
| Typed input request | 392 B | 96 B |
| Published status object | 64 B | 48 B |
| Map with revision and alignment | 32 B | 16 B |
| Owner context | 544 B | 504 B |

The message buffer is byte-bounded, not count-bounded. Mixed/small frames can
therefore queue more requests than the old four fixed-size slots: up to 28 mode
requests, 14 mapping requests, 12 odometry updates or 21 completion requests
when the buffer contains only that kind. The output buffer can hold 16 navigation
frames or 19 replies. The owner still processes at most four inputs per cycle.
This changes backlog/overload thresholds and must be included in target latency
measurements; four maximum-size inputs is a capacity guarantee, not an exact
four-message limit. Do not flood configuration/mode requests, and do not treat
successful enqueue as accepted mode or completed motion. The dedicated motor
emergency-stop path remains outside this ordinary S3 queue.

Queue requests save **1,568 bytes**. Allocator metadata is excluded; heap backing
memory may remain reserved in the link map. Both task stack allocations remain
8 KiB. Smaller objects are not a claim of smaller allocated task stacks.
Bit-field packing does not necessarily reduce `sizeof`: several algorithm
objects keep their original size because of alignment.

The compiler reports owner-task frames of 1,280 -> 1,016 bytes and submit frames
of 8 -> 96 bytes; codec decode adds its own frame. These are individual frames,
not complete call-chain or measured stack high-water values. Use the saved
`.su` contents in the measurement JSON for the complete per-function report.

## Work required from other owners

| Owner | Required action |
|---|---|
| Controller/integration | Rebuild all sources with the new S3 headers. Existing production calls compile unchanged. Use the three S3 APIs; never send native structures directly to `mbf_s3_input/output`. Poll status and preserve freshness, raw/filter agreement and fault checks. |
| Buddy 1 | Existing telemetry still works as before, but JSON does not meet the new binary requirement. Agree a versioned binary schema with the dashboard/client, including bit allocation, units, validity and byte order. For a text map boundary, call `nav_update_parse(&request.value.mapping, text)` and check its result before submitting. Raw MQTT bytes are never a C structure. |
| Buddy 2 | No motor, encoder or odometry source change is required by this patch. Preserve uint64 raw-time S3 odometry and forward micrometre semantics through the controller, plus matching completion IDs. Audit motion/encoder representations separately; do not narrow values without measured range/overflow policies. |
| Buddy 4 | No IMU-source change is required by this patch. Keep using `app_stats` under `mtx_stats` in the retained monitor. Audit IMU/hump fields separately and coordinate any shared fault allocation. |
| Buddy 5 | No obstacle-source change is required by this patch. Continue bypass/recovery coordination through the controller; LINE_FOUND still means sustained front-pair confirmation, not independently verified original-route identity. Audit obstacle/scan representations separately. |
| All owners | Do not index packed `status.mapping` or `map.actions` as an enum array. Use `nav_action_get(..., letter - 'A')`. Use named fault constants, not old numeric ordinals. Do not take addresses of bit-fields, persist native layouts or mix previously compiled objects. |

Example mapping request, after parsing and validating any transport envelope:

```c
s3_input_t request = {0};
request.kind = S3_INPUT_MAPPING;
request.request_id = request_id;
if (nav_update_parse(&request.value.mapping, text))
{
    ER result = s3_submit(&request, TMO_POL);
    /* Handle enqueue failure; E_OK still requires an accepted owner reply. */
}
```

An invalid binary update is queued as a rejection marker so the owner can emit
the correlated negative reply. Parsing text must still be checked by its caller.
Updates preserve unselected letters and are committed all-or-none. Existing
startup mappings and snapshotted pending manoeuvre actions are unchanged.

## Synchronisation and remaining evidence

Only the S3 owner mutates its context/map. Published status is copied under the
existing inheritance mutex, with `TMO_POL`. Queues copy encoded bytes. No ISR,
priority, affinity, tick, network-service ownership, motor access or startup
ordering changes. Fault values now identify condition bits; existing first-fault
latching and output-full stop/pause handling remain, rather than introducing
concurrent writers to a global vehicle fault word.

Host tests cover packed storage, malformed frames, correlated rejection,
sample-gap faults, overload, recovery and controller/provider integration.
Normal and TEST=1 Make builds are compile/link evidence only. The target
comparison uses GNU Make and the already-installed SDK 2.3.1. WSL Ubuntu was
unavailable, so the existing native MSYS GNU Make and Arm GNU 10.3.1 were used.
No source/build-system changes, tool installation, dependency fetching, flash
or hardware actuation were used to make those builds work.

The identical-profile normal ELF comparison and exact commands are recorded in
`testing/subsystem3/TEST_LOG.md`. Flash cost increases because of validation and
serialization; this is a measured tradeoff, not an unqualified efficiency claim.

Pending S3-U31: measure API/codec execution time, sample-to-motion latency,
sampling jitter, complete stack high-water and stop response on the robot under
SMP/network load. Keep calibrated timings and current stop/freshness policies.
Successful host tests and compilation do not establish acceptable target timing,
physical barcode speed, electrical safety or complete Week 13 compliance.

# Member 3 integration with the professor's template

> Current contract, 2026-10-26: [Barcode demo](../barcode-demo.md) supersedes
> earlier startup defaults, partial production maps and immediate barcode
> manoeuvres below. Production starts with no mapping, receives four letters
> over MQTT, and acts on the next junction. Historical evidence is retained.


> Follow-up safety review: see [feedback audit and current contract](feedback-audit-and-contract.md)
> for checked delay failure, identifier exhaustion, odometry lifetime and current
> evidence. Earlier size/stack/flash figures below identify the packing release;
> the appended feedback run in TEST_LOG records the later firmware comparison.

> Current update 2026-10-21: `vehicle.c` and `vehicle_rt.c` now implement the
> controller caller. Older statements below saying it is absent are historical.
> Queue layouts, packed mapping access, mapping requests and fault encodings are
> superseded by [minimum-bit representation](minimum-bit-representation.md).
> Existing configuration/mode/odometry/completion calls remain source compatible;
> every component must be rebuilt with the current headers.

## Delivered layout and scope

`app_program/subsystem3/` is a self-contained replacement/addition for the
professor's Subsystem 3 folder. The complete template foundation is now imported
into this repository, so build directly from the repository's `build_make/`.
The integration was first overlaid onto a downloaded template in
`build/template-reference/INF2004_ProjectTemplate-main/` and built with the
template's real makefiles, kernel, headers, USB console and radio service.

Only Subsystem 3 application files were changed in that checkout. Subsystems
1/2/4/5, shared app files, kernel configuration and template makefiles were
preserved. The existing repository's Pico SDK/Member 5 executable is preserved;
its CMake target does not run the RTOS application. Use the template make build
to run the new Member 3 task. Subsystems 1/2/4/5 are the professor's original
examples, not the teammates' completed robot integrations.

The original S3 consumer, mailbox, statistics and mutex remain because the
template's S2 producer and S4 monitor use them. S3 initialization/start also
creates/starts the navigation task. This preserves the unchanged example
subsystems; the example consumer does not command the robot. Retire the example
interfaces later only when their owners remove those dependencies.

The algorithm files (`drv_*.c` and `line/*.h`) are standalone copies of our
existing tested line/barcode library. Headers use relative local includes;
the barcode count parameter uses uint32_t so importing the public interface
does not conflict with this port's size_t typedef. Updates to the original
algorithm files must also be applied to these template copies and tested.

## Install and build

No install step is needed in this branch now: it already contains the full
[template](https://github.com/sirfonzie/INF2004_ProjectTemplate) and Member 3.
From the repository root, enter `build_make/` and run `make -j4` using the toolchain
setup in the root README. The old CMake build remains available only for the
standalone modules/bench program.

To export Member 3 to a different existing template checkout, first commit or
back up that checkout's Member 3 work, then run from this repository:

```powershell
./tools/install-member3-template.ps1 -TemplateRoot 'C:/path/to/INF2004_ProjectTemplate'
```

The installer copies files only into subsystem3 and checks SHA256 hashes of
the other four application folders. It never deletes files. The original
app_main already calls subsystem3_init/start; no shared-file edits are needed.
In the template's build_make folder, use its documented build environment:

```sh
make -j4
```

Successful output is `build_make/mtk3pico_smp1_usb_cdc_wifi.uf2` for the default profile.
Do not confuse this with the older standalone `member3_ir_diagnostic.uf2`.
Neither firmware has been flashed or tested on the robot by Codex.

## Runtime behavior

- Starts paused and uncalibrated, with no letter-to-movement assignments.
- Reads GP2 left, GP4 right and GP27 side through a single owner task.
- Configuration, mapping, mode and odometry messages are processed serially.
- Reports filtered line changes, barcode characters and snapshotted actions,
  junctions, line loss, line reacquisition and faults.
- Mapped junction actions enter AWAITING_MANOEUVRE; no new actions are generated until
  the controller acknowledges the matching event ID.
- Manoeuvre completion starts line recovery. Stable BOTH-BLACK emits LINE_FOUND
  and leaves the module paused until the controller explicitly resumes tracking.
- Bypass/paused modes keep sensing without triggering unexpected-line-loss events.
- Invalid mappings produce a rejection reply; mappings already attached to an
  issued manoeuvre do not change when the table is updated.
- Mapping settings are RAM-only. No motors or network APIs are called.

## Public integration contract

Include `member3_navigation.h` from subsystem3. All APIs are task-context only.

| Interface | Caller / purpose |
| --- | --- |
| `s3_submit(&input, TMO_POL)` | Queue a configuration, mode, mapping, odometry or manoeuvre-completion message |
| `s3_receive(&output, timeout)` | Single controller receiver drains S3 events and validation replies |
| `s3_get_status(&status)` | Poll once for coherent copied status; E_TMOUT on contention; any error leaves output unchanged |

E_OK from submit means enqueued, not validated. S3_OUTPUT_REPLY echoes request_id,
accepted and map revision. Odometry is high-rate and does not produce per-packet
replies. Send a complete zero-initialized s3_input_t and a validated binary four-letter
mapping update. Raw network bytes are not a C structure; Member 1 must parse
and validate its transport envelope before constructing one.

The status getter requests `TMO_POL`, never `TMO_FEVR`, and never retries
internally. E_OK confirms copying, not fresh sensing or motion readiness.
`raw.timestamp_us` is the original acquisition time. `black_mask` is raw
normalized sensing; `line` retains the last stable filtered observation and
may disagree during settling. Check all validity/mode/fault/pending-ID fields
and sample age. E_TMOUT means try again on a later supervisory cycle while
applying the controller's unavailable-feedback stop/restriction policy.
The owner also polls for publication: contention skips publication, preserves
the previous sample timestamp and continues sampling. Actual SMP execution
latency remains to be measured; this is not a physical emergency-stop proof.

Configuration is accepted only while paused. Supply actual calibrated polarity
and measured timing. `s3_config_t` fields include line stability/loss/found
intervals, recovery timeout, barcode quiet/timeouts/rearm interval and sampling
gap limit. Junction detection requires its measured spatial configuration and
fresh cumulative forward odometry from Member 2. Junction may be disabled while
that interface is unavailable. The legacy USB calibration firmware and existing
ir_observe/ir_calibrate functions provide calibration evidence; the RTOS task
does not guess polarity or sensitivity.

Set MODE=TRACKING only after configuration acceptance. For intentional obstacle
bypass, send MODE=BYPASS, then MODE=RECOVERY after bypass. Do not reset a latched
fault by just sending TRACKING: pause and submit validated configuration to
reset processing. The controller must handle safe stopping; S3 cannot stop motors.

For a JUNCTION event with command != NONE, execute its snapshotted command,
then submit MANOEUVRE_DONE with that event_id. Wrong/stale IDs are rejected.
Completion is accepted only in TRACKING/AWAITING_MANOEUVRE with no fault.
Pause preserves pending ownership and rejects late DONE; failed/cancelled
motion must not send DONE. Reconcile motion ownership before a deliberate
paused reconfiguration clears that pending ID.
NONE means a recognized but disabled letter and requests no movement.

The output buffer is bounded and all task sends use TMO_POL. If a receiver does
not drain it, S3 latches OUTPUT_FULL, pauses navigation and exposes the fault in
status; the controller must poll status even when consuming events. The queue
contains up to 16 outputs; ordinary line changes and replies share that budget.
Startup objects: two message buffers, one mutex, one additional task (8 KB stack)
on top of the retained demo consumer's existing objects.

## Scheduling and calibration limits

See [production timing and handoff](production-timing-and-handoff.md) for the
separate 10 ms host suite, change-only event/continuous-motion refresh policy,
recovery confirmation fix and pending hardware/controller acceptance procedure.

The template's CNF_TIMER_PERIOD is 10 ms. Configuration period must be an
integer multiple of that tick, and max_sample_gap_us must allow the requested
period and real jitter. A request for 1 ms polling cannot provide 1 ms sampling
under this shared kernel configuration and is rejected. If polling cannot
resolve the narrowest barcode element at intended speed, reduce speed or agree
on a future timestamped-edge acquisition design. No shared GPIO interrupt
handler or timer period was changed in this integration.

The navigation task uses priority 17, in the template's existing S3 band, on
core 0. The preserved consumer is priority 16. This is compatibility with the
professor's starter, not a final real-time performance claim; the team must
coordinate final application priorities without students changing each other's
allocations. Driver reads use the RP2040 raw 64-bit timer and SIO GPIO snapshot,
without Pico SDK runtime calls or Wi-Fi service dependencies.

Calibrated polling speed, optical contrast, mounting, junction dimensions and
recovery behavior still require the actual robot. A recovered both-black signal
confirms a line under the sensors; mission context must establish it is the
original route. Junction detection retains the documented side-width limitations.

## Verification performed

- Default dual-core template built and linked from this repository's own
  build_make directory; UF2 generated. All 643 template source files are present.
- Host fake-kernel tests exercise the delivered owner-task logic for runtime
  mapping validation, barcode-to-command output, correlated manoeuvre completion,
  line-loss/reacquisition, recovery timeout, bypass suppression, sample gaps and
  output-buffer-full fault handling. These simulate kernel calls, not actual SMP
  scheduling.
- Existing IR, barcode/navigation and junction algorithm tests remain available.
- Unchanged Subsystems 1/2/4/5 and shared app/kernel/build files verified against
  the downloaded template source.

Pending: live consumers/producers in Members 1/2/5 and the Mission Controller,
on-board RTOS/scheduling tests and hardware calibration. Their files have not
been edited to create these connections. The integration supplies the S3 side.


## Buddy 1 mapping reply contract - 2026-10-26

For every processed S3_INPUT_MAPPING request, S3 emits S3_OUTPUT_REPLY with
exactly the incoming 32-bit request_id. Buddy 1 uses `0x80000000 | seq`;
S3 preserves the high bit and sequence, without replacing them with event_id.
The packed input and reply formats both carry all 32 bits. S3 treats this ID
as an opaque correlation value and does not construct Buddy 1 sequences.

The reply's accepted flag reports whether installation succeeded, and
map_revision identifies the resulting installed revision. Invalid/incomplete
maps and maps blocked by a pending barcode/manoeuvre receive accepted=0 with
the original request ID and unchanged map/revision. Buddy 1 remains responsible
for reporting ACCEPTED/REJECTED to the dashboard from that reply; successful
queue admission alone does not confirm installation. S3's existing output-full
fault/drop policy still applies if a reply cannot be queued.

S3-U40 tests IDs 0x80000000, 0x80000001, 0x80123456 and 0xffffffff through the
public submit/receive packed transports and real S3 owner dispatch, including
accepted maps, incomplete-map rejection and pending-instruction rejection.
This S3-only update changes no wire layout, Buddy 1 code or shared controller.

# Buddy 3 production timing and controller handoff

> Trigger confirmed by the user on 2026-10-21: after full barcode validation,
> coordinate the controller stop/manoeuvre handshake without waiting for a
> junction. Retain freshness and stop/fault/obstacle arbitration. See the
> current contract for the remaining physical validation and mapping checks.

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

## Status and evidence

This is the S3 provider contract and a **proposal for controller integration**,
pending agreement with Buddies 2/5 and integration. The repository has no Mission
Controller or connected production motor consumer. No motor, shared startup,
kernel, build configuration or other subsystem implementation is changed here.

Repository finding: Buddy 2 now has `motion_api.h`, continuous refresh/expiry
and a motion task, alongside its retained demonstration producer. Root README,
TEAM and older S3 integration text still describe S2 as examples only; those
descriptions are stale. The motion API explicitly labels itself PROVISIONAL.
Its presence does not establish a connected S3/controller consumer or agreed
final interface. Shared documentation is left for coordinated owner updates.

The professor's write-up requires line tracking, barcode-directed encoder-based
motion and original-route reacquisition. Week 6 FR1–4, FR14, FR21 and NFR2/4/13
require the corresponding sensing, recovery and coordination. The Week 6 task
table mentions left/centre/right IR, whereas its GPIO table and this code use
left/right front sensors and a side barcode sensor (GP2/GP4/GP27). Treat the GPIO
layout as the implementation finding; confirm actual mounting before driving.

The professor's write-up page 3 gives an **example** A=LEFT, B=RIGHT, C=STRAIGHT,
D=UTURN. Existing startup code uses A=FORWARD, B=LEFT, C=RIGHT, D=UTURN; other
letters are NONE. The requesting user confirmed the existing mapping on
2026-10-15. Integration should submit `MAP A=FORWARD B=LEFT C=RIGHT D=UTURN`
and validate its reply/revision before movement.
The sample mapping is not an instruction to silently change the startup table.

## Production-tick verification

`config/config.h` sets `CNF_TIMER_PERIOD=10` ms. S3 accepts only 10..100 ms
integer multiples and a maximum sample gap at least as long as the period.
The task delays **after** bounded input work, sensor processing and status
publication. Requested delay is not the measured acquisition interval: task
execution, tick phase, preemption and mutex contention affect it. A 20 ms period
being accepted does not mean it is suitable for a particular barcode or speed.

Run `testing/subsystem3/verify_production_timing.py` from the repository root
with an installed host GCC. It runs the retained 1 ms logical suite, the three
algorithm suites and a separate production suite. The production suite checks:

- All accepted tick multiples and representative unsupported periods.
- Independent physical edge timelines sampled at 10/20 ms, phase variation,
  forward/reverse *A*, and synthetic 10/11/12 ms acquisition jitter.
- Gap budget boundary, excessive gap, reversed timestamps, undersampled frames
  and a pulse completely hidden between samples.
- The actual owner-task loop under persistent input load: four messages per
  cycle and continuing sensor reads, including with an undrained output queue.
- Wrong/matched/duplicate completion, bypass, timeout, invalid sensor payload,
  unchanged line observations and explicit resume.
- Interrupted reacquisition: current raw non-centred sensing breaks LINE_FOUND
  confirmation even while the filter retains its previous centred observation.

These are host simulations using fake kernel calls and synthetic timings.
The load fake supplies an always-ready input stream and models bounded output
acceptance; it does not emulate kernel queue scheduling, mutex contention,
SMP execution or driver electronics. The 60 ms narrow/180 ms wide fixture is
deliberately resolvable, **not a measured operating envelope**. The retained
logical suite remains at 1 ms; no shared timer setting is changed.

## Physical envelope worksheet — S3-U19, not yet run

The supplied Barcode & Line Specification page shows an 18 mm line, 20 mm
barcode-to-line gap and approximately 141 mm barcode length. It does not supply
a verified minimum element width or optical response measurement. Do not derive
the maximum speed from the overall printed length alone.

Record the following with revision, configuration, instrument evidence and
operator/date. Use the actual track print and all supported scan directions.

| Measurement | Current evidence / required record |
| --- | --- |
| Narrowest black AND white element, internal gaps, wide:narrow ratio | Unmeasured; record minimum physical widths and print variation in mm |
| Quiet zones on both ends | Unmeasured; measure usable white lengths in mm |
| Sensor model, supply, DOUT voltage and ground | Unverified; verify compatibility before connecting; board digital-high limit is 3.6 V |
| Mounting height, lateral/longitudinal position, yaw, optical footprint | Unmeasured; record effective footprint and response across edges, not PCB centres |
| Black/white polarity and stability of all three channels | Unmeasured; save raw stationary captures at actual mounting and lighting |
| Actual speed, ripple/acceleration, scan direction | Unmeasured; record measured speed, not just requested motor speed |
| Acquisition interval minimum/maximum, jitter, worst gap under load | Unmeasured; capture boot-relative timestamps with USB/Wi-Fi/other tasks active |
| Recognition trials per character/direction/speed | Unmeasured; record attempts, correct decodes, misses, invalid frames and wrong commands |
| quiet/timeout/rearm and line filter/loss/found settings | Unvalidated; record values, justification and observed errors |

For an ideal point sensor at constant speed, element dwell is width/speed.
If a dwell fits between two acquisitions, the transition can be invisible;
neither valid timestamps nor a gap watchdog can detect that loss. Real optical
footprint, hysteresis and speed variation further affect observed run widths.
Choose a lower trial speed first and measure margins. Timestamped GPIO edges
are a future acquisition proposal only if measured polling remains inadequate.

For each trial record the longest observed internal white run and the shortest
usable quiet zone. Choose quiet time above internal white runs with acquisition
uncertainty and below usable external quiet time. Choose frame timeout above
the slowest measured complete frame plus trailing quiet-zone confirmation.
Validate rearm against repeat scans and legitimate successive barcodes. A
single fixed timing configuration may not span the full speed range; record
the tested range and reject operation outside it. Do not label a simulated
fixture, calculated ideal limit or one successful traversal as calibrated.

Define the required recognition success rate, number of repeat trials and
acceptable timing/stop limits **with the team before acceptance testing**.
Repeat on every A–Z character actually used, in both scan directions, at speed
boundaries and under lighting/height/print variation. Preserve failure records.
Add authorised hardware-test video links to `videos/README.md` when available.

## Proposed line-to-motion policy

Left/right are the vehicle's perspective facing forward. Normalized bits 0/1
are the front left/right; bit 2 is the side sensor. Confirm that at the centre
of the actual 18 mm track both effective front sensing points read black.
If both are white when centred, this mounting does not satisfy the classifier.

| Front mask | S3 observation | Proposed controller response |
| --- | --- | --- |
| 3 | CENTRED | Refresh calibrated straight wheel speeds |
| 1 | CORRECT_LEFT | Refresh calibrated left correction through Buddy 2 |
| 2 | CORRECT_RIGHT | Refresh calibrated right correction through Buddy 2 |
| 0 | ABSENT | Stop or use a separately agreed restricted recovery policy; do not refresh ordinary forward motion |

S3 provides direction categories, not a continuous lateral position or chosen
wheel-speed magnitude. Both black can also be a junction. Buddy 2 owns all PWM,
direction outputs, speed/turn calibration and continuous command expiry.

The controller must run a periodic supervisory cycle independently of change
events. Each cycle reads `s3_get_status()` and checks all of:

1. Successful API result; configured/calibrated and `line_valid`.
2. `fault == S3_FAULT_NONE`, intended TRACKING mode, and permitted tracking
   state (TRACKING/BARCODE_DECODING), with no pending manoeuvre ID.
3. `raw.timestamp_us` is not in the future and its age is within an agreed
   sensing freshness limit, on the same boot-relative microsecond clock.
4. Raw and filtered front patterns satisfy the agreed transition policy.
   The filter retains the old observation during settling. Fresh raw time
   alone does not prove the old filtered direction remains appropriate.

Only then refresh the requested wheel speeds through the agreed Buddy 2 API.
`S3_OUTPUT_LINE` is change-only, not a heartbeat. A centred line may generate
one event while `status.raw.timestamp_us` continues advancing. Reusing an old
queued line event to refresh motion defeats freshness checking.

The current provisional `motion_api.h` requires continuous refresh with the
SAME motion ID and newer `issued_us` before expiry, confirmation through
`last_request_id`, `last_request_issued_us` and `b_request_accepted`, and terminal
acknowledgement before a higher ID. QUEUED is intake success, not execution.
Correlate motion IDs separately with S3 event IDs; do not assume the counters
are interchangeable. Handle BUSY/UNAVAILABLE/ERROR without renewing motion from
an unchecked old observation. These are inspected provider semantics, not a
new cross-subsystem agreement.

S3 acquisition uses the RP2040 raw microsecond timer; `motion_api_now()` uses
kernel monotonic operating time at 10 ms resolution. Do not subtract S3 raw
timestamps from motion time or copy them into `issued_us` without establishing
the clock relationship. Use `s3_clock_us()` for S3 age, and the owning motion
clock for command issuance; agree any conversion explicitly.

Agree controller period, maximum response latency, sensing-age limit,
transition restriction and Buddy 2 command lifetime together. Measured worst
refresh interval plus transport/dispatch margin must stay below that lifetime.
Stop/restrict on failed reads, stale/future sensing, fault, invalid sensing,
unexpected modes or controller overload. Do not allow ordinary queued steering
to override a stop. Review and hardware-validate a prioritised bounded stop path
that preserves Buddy 2 motor ownership and does not depend on telemetry or a
full S3 output queue. Status reads now poll once (`TMO_POL`): `E_TMOUT` means
contention, `E_PAR` means NULL output, `E_OBJ` means unavailable objects, and
other kernel errors pass through. Every error preserves the caller's buffer,
including an unlock error. Never treat that old buffer as a new successful
read. E_OK means a coherent copy, not permission to move. No timeout parameter
or public structure layout changed. A controller can use the existing getter
without requesting a wait; there is no internal retry loop.

Owner publication also polls once. A busy mutex skips publication and leaves
the last published acquisition timestamp untouched; sampling continues and
the consumer must age out that snapshot. Unexpected lock/unlock errors latch
a kernel fault. With an unusable mutex, a new fault snapshot might not be
publishable, so the controller's independent age/error policy remains essential.
Neither polling nor a coherent copy establishes a target response-time bound:
kernel critical sections, SMP preemption and the physical stop path still
need measurement. No motor-stop behavior is implemented by this getter.

## Existing manoeuvre and bypass sequence to connect

1. Drain outputs and also poll status, even when no events arrive. Treat
   `s3_submit(...)=E_OK` as enqueue success only. Match `S3_OUTPUT_REPLY` using
   `request_id`; require `accepted` before relying on a mode/config change.
2. For BARCODE_VALID use the event's snapshotted command, map revision and
   event ID. NONE requests no manoeuvre. Execute each event once; mapping
   updates do not rewrite an in-flight command. Route motion through Buddy 2.
3. Stop ordinary line-following refresh during AWAITING_MANOEUVRE. Correlate
   the Buddy 2 motion request/completion with the originating S3 event ID.
   Send MANOEUVRE_DONE only for successful completion of that manoeuvre;
   wrong/duplicate IDs are rejected. A failed/aborted motion needs controller
   stop/fault policy, not a false completion acknowledgement.
   DONE additionally requires TRACKING/AWAITING_MANOEUVRE and no latched fault.
   Pausing a pending manoeuvre preserves its ID but rejects a late matching
   DONE; it cannot enter recovery or resume navigation. Reconcile/cancel the
   motion first, then deliberately reconfigure S3 while paused if abandoning
   that pending operation. There is no separate S3 cancellation transport yet.
4. Matching completion enters RECOVERY. Before an intentional bypass request
   MODE=BYPASS and wait for acceptance; sensing continues without line events.
   Coordinate any outstanding barcode manoeuvre first: switching mode does not
   cancel its pending event. After bypass request MODE=RECOVERY and validate
   the reply. Do not resume TRACKING directly from BYPASS.
5. Recovery requires filtered CENTRED and uninterrupted raw both-black samples
   for `line_found_us`; interruption resets confirmation. Only sampled evidence
   is known. On LINE_FOUND S3 pauses. This confirms a line under the sensors;
   mission route context must identify the original line.
6. After original-route confirmation explicitly request TRACKING. That resets
   filter validity; wait for fresh valid sensing before ordinary motion refresh.
   Recovery timeout or sensor/queue fault requires stopping and diagnosis.
   Reconfiguration while paused clears faults and pending manoeuvre state;
   reconcile/cancel controller motion ownership before using it as a reset.

## Controller-connected acceptance — S3-H02, not yet run

With the actual controller and Buddy 2 connected, define stable integration
IDs with their owners and record exact tested revisions/configurations:

- Hold the line centred with no observation changes: motion refresh continues;
  stop refresh and verify Buddy 2's command expiry at its agreed deadline.
- Suspend/stall S3 sampling: controller rejects ageing status and stops within
  the agreed bound even though S3 cannot report its own stall.
- Fill output queue: OUTPUT_FULL/PAUSED is detected through status and motion
  stops despite a dropped fault event. Confirm input load does not starve reads.
- Inject sensor payload errors, disconnect/stuck-high/stuck-low sensor conditions
  under controlled conditions: verify controller restrictions and stop latency.
  Static valid digital levels alone cannot diagnose a disconnected/stuck sensor;
  malformed-payload host tests do not establish this coverage.
- Reject wrong/duplicate completion; test aborted turns and bypass overlapping a
  pending barcode. Confirm no motion is resumed by a stale LINE_FOUND event.
- Interrupt recovery with white/corrections, run recovery timeout, and reacquire
  another line: no ordinary tracking until mission context validates the route
  and a new TRACKING request plus fresh sensing is accepted.

Hardware execution, firmware flashing and cross-subsystem implementation are
pending authorised team integration. This document specifies the remaining work;
it is not evidence that these tests passed.

## Configuration and mapping delivery proposal

These steps use existing provider interfaces; integration must implement and
approve the sender. No connected controller or configuration sender was found
in this checkout. Do not send bytes directly from MQTT into an S3 structure.

1. Keep motion disabled/stopped. Obtain polarity using the existing black/white
   calibration procedure at the measured sensor placement. Supply the complete
   `s3_config_t` from an explicitly selected, version-controlled calibration
   record; no universal default thresholds, timing or speed are supplied.
2. One controller owns delivery. Submit PAUSED using a unique `request_id` and
   `s3_submit(..., TMO_POL)`; wait for its matching accepted reply while draining
   unrelated outputs. Handle queue-full/timeouts/errors without enabling motion.
   Use a finite controller deadline, chosen with the motion-refresh budget;
   S3 cannot choose that deadline on the team's behalf.
3. Submit CONFIG in another zero-initialized request. Require its matching
   accepted reply. Rejection leaves the previous configuration intact; deliberate
   successful reconfiguration clears faults and pending manoeuvre ownership.
   Account for that reset before using it to recover a fault/cancelled turn.
4. Submit the agreed full mission mapping using MAPPING and another request ID;
   omitted letters retain their existing mappings, so explicitly set unused
   letters to NONE when applying a complete mission map. Require the accepted
   reply and record `map_revision`. Parse FORWARD as the straight action.
   Invalid mapping updates are transactional and leave the old map/revision.
5. Confirm the published map/revision, calibration, intended mode and fault.
   CONFIG/MODE acknowledgements are not proof that a new sensor sample exists.
   After the accepted reply record `s3_clock_us()`; require a subsequently
   acquired/published sample newer than that checkpoint (allow the 10 ms tick
   and measured scheduling jitter). Then request TRACKING, require its accepted
   reply and again wait for fresh `line_valid` with raw/filtered agreement.
   Filter reset intentionally invalidates old steering evidence.
6. Only the controller may use that validated feedback to submit motion to
   Buddy 2. On a failed/expired operation, reconcile its actual terminal result;
   enqueue/admission success does not justify MANOEUVRE_DONE.

The requesting user confirmed A=FORWARD, B=LEFT, C=RIGHT, D=UTURN on 2026-10-15.
This matches the retained startup map; the professor's differing map is an
example. Deliver the confirmed map explicitly as above. The provider can accept
other maps through its existing validated interface; an alternate-map test
checks delivery/transaction semantics and does not change the mission choice.

## Controller-owned mission completion: proposed end-of-line cue

No S3 event currently means MISSION_COMPLETE. LINE_FOUND, an unmapped letter
(NONE), a junction or line absence must not be interpreted as finishing. A
telemetry robot-state enum elsewhere is not a navigation completion source.
The requesting user selected controller route completion, then suggested
"until there is no black line anymore?" on 2026-10-15. Record end-of-line as
the proposed cue, not a verified detector or permission to finish on every
LINE_LOST event. Both-white sensing cannot distinguish a track endpoint from
accidentally leaving the line, a gap, poor contrast or invalid sensing.

Proposed controller acceptance rule: route context first establishes the
expected final segment/finish region; fresh, calibrated, fault-free raw and
filtered absent observations then persist for a measured finish interval.
The controller stops through Buddy 2 and confirms successful stopped motion
before publishing mission completion once. Invalid/stale sensing, failed stop
or an unexpected line loss must not count as finishing. Outside that expected
finish context, keep the existing recovery/timeout policy.

The route predicate that identifies the finish region and the calibrated
absence/stop limits remain unspecified. Do not invent them, relabel S3's
line-loss timeout as finish time, or add a local MISSION_COMPLETE event.
Integration must coordinate final-region arming with the existing transition
from LINE_LOST to RECOVERY. No finish barcode is assigned; DONE, NONE, UTURN
and LINE_FOUND keep their existing meanings. The finish trigger remains
unimplemented until these course facts and controller policies are confirmed.

## Added bounded-feedback verification

The existing runner now also builds `member3_feedback_test.c`: S3-U20..U25
cover polling contention/timeout, lock/unlock errors with unchanged output,
coherent fields, read-without-sampling staleness, continuously fresh unchanged
line, raw/filtered disagreement, sampling during contended publication,
withheld and paused completion, barcode noise and configuration/map replies.
S3-U16/U18 continue checking wrong/duplicate/matched completion, explicit resume
and actual sampled line-reacquisition evidence. All mutex outcomes and execution
cycles here are host fakes; on-target timing and physical calibration remain
pending. No real controller/motor connection is claimed by these tests.

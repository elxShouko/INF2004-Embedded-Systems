# Buddy 3 feedback audit and current navigation contract

> Current contract, 2026-10-26: [Barcode demo](../barcode-demo.md) supersedes
> earlier startup defaults, partial production maps and immediate barcode
> manoeuvres below. Production starts with no mapping, receives four letters
> over MQTT, and acts on the next junction. Historical evidence is retained.


This review checks the supplied follow-up feedback against the uncommitted
minimum-bit implementation on `d4c91f2`. It distinguishes existing work, new
fixes, current controller behaviour and work requiring physical measurements
or a team decision. It does not declare the car calibrated or the Week 13
requirement fully satisfied.

## Feedback disposition

| Feedback item | Finding and action |
|---|---|
| Compact sensor/state/navigation representations | Already implemented for S3 queue fields, published status, action maps and owner flags. Native validation carriers and legacy examples remain explicit coverage gaps. |
| Large mapping variant | Already replaced: request is 96 target bytes, previously 392. The mapping payload is a binary partial update, not a 384-byte string. |
| Message layouts and allocations | Already reviewed against real Make/ARM ABI and kernel message rounding. Input/output buffers request 336/384 bytes. Queue capacities are byte-based; small-message backlog limits differ from the previous fixed messages. |
| Raw versus filtered validity | Already documented and tested. Fresh acquisition does not make an old retained filtered observation suitable for steering. The controller checks raw/filter agreement. |
| Queued intake versus validation | Already implemented. E_OK from submit means copied intake; configuration/mapping/mode/completion require accepted correlated replies. Odometry has no per-message reply. |
| Delay return ignored | Confirmed defect, fixed now. Any non-E_OK delay latches KERNEL, pauses, attempts fault output/status publication once and exits the navigation task. |
| Output overflow and counters | Existing overflow pauses/faults. New fix preserves existing fault bits and saturates the drop counter instead of wrapping. |
| Event-ID lifetime | New fix reserves UINT32_MAX for a terminal fault and never issues wrapped/reused IDs. Exhaustion cannot be cleared by reconfiguration; restart required. |
| Timestamp width | Full uint64 raw microseconds retained, with full uint64 cumulative distance. New tests cross the 32-bit microsecond boundary and reject future evidence near uint64 limits. |
| Odometry usability | New receive-side age/monotonic/duplicate validation. Bad evidence disables junction eligibility until a fresh consistent update, while retaining the previous monotonic baseline. |
| Wrong or late completion | Already rejected unless ID, mode, pending ownership and fault state match. Rechecked at ID exhaustion. |
| Before/after navigation equivalence | Added matching 40,316-sample / 352-output traces against the saved pre-follow-up working tree, in normal and optimised host profiles. Intentional new failure responses are tested separately. |
| Latency evidence | Existing and new tests are logical host timing/ordering tests. Same-update event generation is checked, but physical sample-to-motion latency, SMP jitter and WCET remain unmeasured. |
| Actual allocation evidence | Existing evidence measures target ABI, exact queue-create requests and linked RAM. Added TEST=1 S3-U37 kernel buffer readback; it has been compiled but must still be run on an authorised target. Heap allocator overhead/high-water remains unmeasured. |
| Polarity, mounting, black/white calibration and speed/lighting envelope | Still physical work. No values or hardware-test results were invented. |
| Mapping/course rule, junction trigger and original-route identity | User confirmed on 2026-10-21 that a fully decoded mapped barcode triggers the controller stop/manoeuvre handshake without waiting for a junction. Physical course mapping and original-route identity remain to verify; no controller or teammate code was redesigned. |

## Sensor and mapping contract

| Input | Current software role | Still to verify physically |
|---|---|---|
| GP2 | Front left digital line sensor, normalised mask bit 0 | Wiring, voltage, polarity, height, sensing footprint and offset |
| GP4 | Front right digital line sensor, mask bit 1 | Same, including separation relative to tape edges |
| GP27 | Side barcode/transverse-stroke sensor, mask bit 2 | Side, orientation, distance from front pair and barcode/course alignment |

These are the current code assignments and previously recorded user assignments,
not a new physical wiring certification. The code does not have a centre sensor.
It expects both front sensing points to read black when centred. Masks 3/1/2/0
produce CENTRED/CORRECT_LEFT/CORRECT_RIGHT/ABSENT. Confirm that geometry on the
actual track before authorising movement.

Calibration supplies opposite stable black/white references for all three
channels. A configuration acceptance reports validated supplied data, not a
measurement that the sensors have been calibrated on the robot. Heights,
offsets, optical thresholds and timing/speed limits must come from repeatable
physical tests on the actual course material.

The existing startup map remains A=STRAIGHT, B=LEFT, C=RIGHT, D=U_TURN;
E-Z are disabled (NONE). The professor's example map differs. Confirm the
actual course map and submit a checked update if necessary; do not reinterpret
this audit as approval to change the map silently. Mapping updates preserve
unselected letters, increment the revision only on success and do not change
commands already snapshotted into pending manoeuvre events.

## Manoeuvre and recovery contract

The current S3 emits a mapped barcode command when the framed barcode has been
validated, including its trailing quiet zone and rearm checks. It then waits
for the matching manoeuvre completion ID. It does not wait for a later junction.

**Confirmed by the user on 2026-10-21:** a fully decoded, valid mapped barcode
triggers the controller's stop/manoeuvre handshake without waiting for a later
junction. This is the required behaviour, not an outstanding trigger decision.
The current controller checks event freshness, requests a stop and coordinates
the motion-owner handshake before issuing the finite manoeuvre. Existing
stop/fault/obstacle arbitration remains in force; recognition does not bypass
those checks. A junction event remains a separate observation.

No firmware behaviour change is required for this confirmation. Physical
barcode placement, stopping distance and the manoeuvre origin must still be
validated for this confirmed rule. The letter-to-command course mapping is a
separate check and was not changed by this confirmation.

A successful correlated DONE enters RECOVERY. Wrong/stale IDs, paused-mode
completion and completion after a fault are rejected. A failed/cancelled motion
must not send DONE. Stable current both-black evidence plus the filtered
centred observation is required for LINE_FOUND; raw interruption resets its
confirmation interval. LINE_FOUND leaves S3 paused until explicit resume.

In BYPASS or PAUSED, sensing continues but ordinary barcode/junction actions
and unexpected LINE_LOST generation are suppressed. Enter RECOVERY after an
intentional bypass rather than treating the expected off-line portion as a new
tracking failure. S3's sustained line observation cannot establish that the
line is the original route. Buddy 5/controller need independently justified
route identity and clearance evidence.

## Lifetime and failure contract

| Information or failure | Defined behaviour |
|---|---|
| Raw acquisition/navigation timestamp | Original uint64 microseconds from the RP2040 raw timer. No narrowing, restamping on read or modulo-32 freshness shortcut. |
| Published filtered line | Last stable category. `line_valid` means one exists; it does not mean it agrees with the current raw mask. Check calibration, state/mode, faults, source age and agreement together. |
| Controller navigation/sensor freshness | Uses the configured `mission.sensor_age_us` in the correct source clock domain; it is not a new S3 hard-coded expiry. |
| Odometry age | Inclusive `odometry_stale_us` boundary, measured from source timestamp at receive and again during junction processing. Future/regressing data is unusable. |
| Duplicate odometry timestamp | Allowed only with unchanged distance and tracking-forward flag while still fresh. A changed distance at the same time cannot fabricate a spatial sample. Duplicates never renew acquisition time. |
| Invalid odometry | Retains baseline timestamp/distance for monotonic validation, marks evidence unusable and prevents junction eligibility. A subsequent fresh consistent update can restore eligibility. It does not stop line/barcode sensing or invent a new motor command. |
| Event IDs | Nonzero, monotonically increasing across every output, not just barcodes. UINT32_MAX is reserved for terminal KERNEL fault output; further outputs are suppressed, never wrapped. Reboot establishes a new ID epoch. |
| Pending manoeuvre | Remains owned until matching successful completion or deliberate permitted reset. Controller transition timeout, not ID bit width, bounds the mission wait. |
| Request IDs | Opaque uint32 sender tokens echoed by replies. They are not authentication/replay protection. Sender must avoid ambiguous reuse while replies are outstanding. |
| Mapping revision | uint32, no wrap: further updates rejected at UINT32_MAX without changing the map. |
| Output drops | Saturating uint32 diagnostic; no wrap into a falsely healthy-looking zero. |
| Output full | Latches OUTPUT_FULL in addition to existing cause bits, pauses navigation and reports through copied status when publication is possible. |
| Malformed queued bytes | Existing exact-length/padding/range checks reject them; owner latches KERNEL and pauses. Invalid application requests that have a canonical rejection encoding receive negative replies. |
| Status publication busy | POL attempt only; old snapshot retains old time and ages out. No reader is allowed to treat repeated successful copies as new acquisitions. |
| Periodic delay failure | Latch KERNEL, attempt fault event and copied status once, then `tk_ext_tsk()` and defensive return. No retry loop and no unpaced next sample. Restart firmware to restore this owner. |

If the output queue is full and status mutex busy during a delay failure,
immediate fault delivery cannot be promised. The owner still exits; the last
published acquisition ages out under the existing controller policy. The
controller must monitor freshness and unavailable feedback, including while
executing a manoeuvre. S3 does not directly actuate motors or implement an
independent emergency-stop channel. Target measurements must establish the
resulting stop latency; no such measurement is claimed here.

## Evidence and next target session

New host IDs are S3-U32..U36. The production runner now bounds each subprocess
to 60 seconds, and the failure fake rejects a second failed-delay call so a
regression cannot spin indefinitely. All earlier packing, decoder, recovery,
overload and status tests remain in the suite. The differential baseline is
the saved **uncommitted minimum-bit release**, not bare d4c91f2; its source
hashes are verified before comparison.

S3-U37 adds read-only `tk_ref_mbf()` diagnostics to the existing TEST=1 S3
self-test. On an idle, unconfigured navigation image, save `[S3-MEM]` lines and
the test result. Expected empty-queue free bytes are 336 and 384, with maximum
message sizes 79 and 19. Nonempty queues invalidate that measurement's idle
precondition; do not drain production data to force a pass. This reads kernel
buffer state, not total allocator overhead or whole-vehicle heap high-water.
No on-target result has been collected by this change.

For physical S3-U19/U31 acceptance, first agree the recognition success target,
speed/lighting range and latency/jitter/stop budgets. Record sensor model,
voltage checks, mounting height/separation/offsets, surface/print geometry,
motion calibration, firmware hashes, SDK/profile and measurement instrument.
Measure acquisition -> filter confirmation -> event enqueue -> controller
consumption -> Buddy 2 command acceptance -> physical PWM/motion response in
their respective clock domains. Do not equate a host same-update assertion
with a zero-microsecond physical latency. Repeat before/after under the same
hardware/load conditions, including Wi-Fi load, queue overload and publication
contention. Save distributions, maxima, missed-event counts, stack high-water
and failure/stop traces, rather than only averages.

Teammates' immediate action is to rebuild against the same headers and retain
their current freshness/fault/handshake checks. Buddy 1's binary MQTT migration,
Buddy 2's physical motion calibration, Buddy 5's route identity and verification
of the course letter mapping remain separate responsibilities. No shared/controller,
motor, Wi-Fi, IMU or obstacle production implementation is changed by this audit.

## Buddy 2 handoff clarification — 2026-10-28

The existing S3 contract and packed formats remain unchanged. S3-U41 and
S3-U42 exercise the public packed input path and real owner dispatch.

- S2 requested/measured wheel speeds are signed int32 mm/s; per-wheel
  positions are signed int64 micrometres. Preserve sign and units in the
  controller. S3 consumes cumulative forward uint64 micrometres, not signed
  wheel position or a speed magnitude. Reverse/turn/bypass observations must
  not be fabricated as forward progress.
- S2 measurement timestamps and command times use kernel monotonic time;
  S2 publication time is a separate field. S3 raw/navigation/odometry times
  use the RP2040 raw timer. The controller owns explicit clock conversion
  preserving source age. Reading, retrying or republishing is not acquisition.
- Equal S3 odometry timestamps require identical distance AND tracking-forward
  state. Inconsistent duplicates invalidate eligibility while preserving the
  last accepted baseline; consistent duplicates expire from original source
  time. Only a newer acquisition can carry a changed tracking state.
- S2 owns its motion command/terminal ID and result. The controller must match
  the active command, require successful manoeuvre execution and its required
  stop/standstill condition, then correlate that command with S3's separately
  stored navigation-event ID. Queue admission, cancellation, timeout and failure
  are not successful manoeuvre completion.
- S3_INPUT_MANOEUVRE_DONE carries that S3 navigation-event ID. request_id only
  correlates the reply; it is not completion evidence. S3 checks pending ID,
  tracking mode, awaiting state and fault, then begins line recovery. It has no
  motion-result/standstill fields and cannot independently verify the controller's
  assertion. No new S2-to-S3 completion API or shared interface was introduced.

Scope: S3 header comments, host regressions and S3 documentation only. S2 and
shared controller code are unchanged. These tests preserve the consumer's
contract; they do not validate S2 terminal generation, the controller's clock
conversion, physical standstill or target SMP timing.

## Controller mode handoff preservation — 2026-10-28

The Vehicle Controller's vehicle_read() already reads current line_state,
line_mode and pending_navigation_id from s3_get_status(). Packed navigation
events retain event-time state and command; S3_OUTPUT_LINE is only an
observation/time/mask message. No extra turning-mode enum is needed:

| Transition | Packed navigation meaning | Current status available to controller |
|---|---|---|
| Ordinary line following | Line observation events on changes | TRACKING mode; TRACKING or BARCODE_DECODING state |
| Actionable junction | JUNCTION, LEFT/RIGHT/STRAIGHT/U_TURN, AWAITING_MANOEUVRE, separate S3 event ID | TRACKING mode, AWAITING_MANOEUVRE; controller owns motor execution |
| Successful correlated completion | Controller submits S3 MANOEUVRE_DONE | RECOVERY mode and state |
| Sustained line loss | LINE_LOST with RECOVERY state | RECOVERY mode and state |
| Stable reacquisition | LINE_FOUND with RECOVERY event-time state | Then PAUSED mode, TRACKING state; explicit controller resume required |
| Resume accepted | Mode reply confirms owner acceptance | TRACKING mode/state; fresh filtered sensing still required |

Event-time state and current mode are separate facts. LINE_FOUND does not itself
authorize driving. Re-reading an unchanged snapshot does not renew sensing.
S3-U43 checks all packed command/state combinations and all mode inputs;
S3-U44 checks real owner transitions for all four actions through packed output
and the public status API. Snapshot publication is modeled by the host fixture;
controller execution and actual SMP delivery are outside those S3 tests.
The existing controller/provider path was inspected; no IMU interface or
shared controller implementation change is required by this feedback.

## Motion increment compatibility — 2026-10-28

Disposition of the additional line/barcode/navigation feedback: no S3 runtime
or protocol change is required for this increment. Continue using the existing
motion/controller APIs, subject to the team's existing interface agreement;
this note does not establish a new agreement or finalize provisional APIs.

Continuous differential wheel-speed requests, finite manoeuvres and the motion
command lifecycle retain their existing semantics. S3 supplies line/navigation
observations to the Vehicle Controller; the controller arbitrates motion and
Buddy 2 actuates the motors. Do not introduce direct S3 motor requests or a
second completion channel for this increment.

Successful submission means queued intake, not successful execution or physical
completion. Keep owner acceptance, successful terminal result, required
stop/standstill, motion acknowledgement and correlated S3 navigation completion
as separate steps under the existing lifecycle. Preserve the distinct motion
command and navigation-event IDs described above.

The S3 input/output queues, message kinds, packed field widths and message
lengths are unchanged. Existing S3-U16/U23/U42 completion regressions and
S3-U27/U28/U43 packing regressions remain applicable. The immediately preceding
host run passed those checks; this additional disposition changes documentation
only and does not constitute a new test run or hardware validation.

# Barcode demo contract (2026-10-26)

The user's clarified demo requirements supersede the earlier immediate-turn
rule recorded on 2026-10-21. This change covers Buddy 1, Buddy 3 and the shared
Vehicle Controller. Buddy 2 continues to own every DC motor output.

## PC mapping message

Publish a plain text payload on the existing MQTT command topic, `robot/cmd`
(unless the build overrides `MQTT_UTK_TOPIC_COMMAND`):

```text
MAP W=FORWARD X=UTURN Y=LEFT Z=RIGHT
```

This is an example supplied at runtime, not a built-in letter assignment.
Pairs can appear in any order. Use four different uppercase A-Z letters and
exactly one each of `FORWARD`, `UTURN`, `LEFT` and `RIGHT`. The payload must be
at most 47 bytes (the existing MQTT receive limit); spaces are accepted between
pairs and at the end. JSON,
lowercase, duplicate letters/actions, partial maps, extra pairs, embedded NULs
and trailing newlines are rejected. The entire transaction is accepted or
rejected; no partial update is installed.

For example, from a PC with Mosquitto tools installed:

```sh
mosquitto_pub -h <broker-address> -t robot/cmd -m "MAP W=FORWARD X=UTURN Y=LEFT Z=RIGHT"
```

Send before starting a run. No letters have actions at boot. An accepted map
replaces all 26 entries, disabling every unselected letter, and increments
S3's `map_revision`. The map lives in RAM; resend it after a reboot. A Wi-Fi
reconnect does not itself erase it. A map is rejected while a decoded
instruction or an issued manoeuvre is pending, preserving that instruction.

The radio service remains the sole network owner. It validates and copies four
letters into the existing S1 command queue. The controller constructs a typed
S3 mapping request; a full S3 queue is retried on later controller cycles until
the original command's one-second age limit. Emergency stop still uses its
independent latch before ordinary command processing. Ordinary command-queue
overflow keeps the existing drop policy; the PC must retry a dropped update.

Queue admission is not acknowledgement of installation. The S3 reply carries
`accepted` and `map_revision`; `s3_get_status()` exposes the applied revision
and table. Mapping replies use reserved request ID `UINT32_MAX` in the shared
adapter. Rejected remote mappings preserve the previous map and do not become
vehicle faults. There is currently no MQTT application-level mapping ACK topic;
MQTT publish success alone does not prove the map was applied. Check the S3
status/reply during commissioning before starting the demo.

## Scan, wait, junction, movement, repeat

1. The decoder recognises a framed Code 39 letter A-Z in either direction.
2. S3 emits `NAV_EVENT_BARCODE_VALID` as an observation. For a mapped letter,
   it keeps the first pending letter and its snapshotted action in owner RAM.
3. The car continues line following and junction detection. Another barcode
   does not overwrite an instruction already waiting for its junction.
4. At a confirmed `NAV_EVENT_JUNCTION`, S3 attaches the stored letter/action
   and enters `LINE_STATE_AWAITING_MANOEUVRE`. A junction without a pending
   mapped letter carries `NONE` and requests no finite manoeuvre.
5. The controller stops its continuous motion through Buddy 2's existing
   handshake, then requests the selected finite manoeuvre. It acts on the
   actionable junction event, never on the barcode observation alone.
6. Matching successful completion enters the existing line recovery flow.
   After stable line reacquisition and explicit controller resume, the next
   barcode/junction pair can be processed. The previous instruction is consumed
   once. Wrong/stale completion IDs remain rejected.

Pause/bypass mode changes preserve an instruction waiting for its junction.
A deliberate successful paused sensor reconfiguration clears it, as it already
clears issued manoeuvre ownership. Faults retain the existing stop/interlock
policy; this change does not add automatic fault recovery.

## Build and physical prerequisites

Use the professor's Make application. MQTT remains an explicit profile:

```sh
cd build_make
make NET=lwip MQTT=1 -j8
make NET=lwip MQTT=1 TEST=1 -j8
```

Supply a compatible Pico SDK with its existing lwIP/CYW43/TinyUSB dependencies
and local `config/wifi_secrets.h`. Default `NET=none MQTT=0` firmware cannot
receive mappings over MQTT. No actual credentials are supplied by this change.

The existing production startup still requires a measured `vehicle_runtime_setup()`
profile, power authorisation and fresh mandatory providers. It has no default
calibration/setup caller. Enable and calibrate S3 junction detection and supply
fresh forward odometry; otherwise no actionable junction can be produced.

The existing detector qualifies a transverse stroke using the front sensors,
side sensor and distance. It confirms after side-white clearance; physical
sensor offset, braking distance and the turn origin must be measured on the
actual cross-junction. Its existing limitations in distinguishing track
topology remain. Host tests cannot establish that a detected stroke is the
required physical cross-junction or that the car turns at its centre.

## Verification

New stable cases: `S1-U23` (actual copied MQTT hook), `S3-U38` (empty startup and
complete replacement), `S3-U39` (four actions, both directions, junction wait
and repetition), `INT-82` (controller trigger), `INT-83` (mapping queue retry)
`INT-84` (mapping rejection) and `INT-85` (event-drain precedence). Existing A-Z decoder and safety tests remain.
Exact commands, revisions, outcomes and limitations are recorded under
`testing/subsystem1/`, `testing/subsystem3/` and `testing/integration/`.

The S1 internal copied command grows by four bytes for the four letters; queue
depth is unchanged. S3's public wire layouts are unchanged. Rebuild all owners
with the current headers. Legacy `src/` standalone programs are bench tools,
not the integrated firmware or the demo's mission contract.

Historical equivalence scripts (`verify_barr_refactor.py` and
`verify_feedback.py`) compare against the former behaviour and are not
acceptance tests for this intentionally changed mission contract. Use
`verify_production_timing.py` for current S3 regressions.

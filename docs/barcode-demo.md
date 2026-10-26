# Barcode demo contract (2026-10-26)

The user's clarified demo requirements supersede the earlier immediate-turn
rule recorded on 2026-10-21. This change covers Buddy 1, Buddy 3 and the shared
Vehicle Controller. Buddy 2 continues to own every DC motor output.

## PC mapping message

The 2026-10-26 overlap cleanup retains Buddy 1's `s1_map` gateway as the
only transport. This section supersedes the original separate command-queue
path and its no-ack description. Use the default binary wire profile
(`S1_WIRE_JSON=0`); legacy JSON mode does not receive runtime MAP messages.

Publish a plain text payload on the existing MQTT command topic, `robot/cmd`
(unless the build overrides `MQTT_UTK_TOPIC_COMMAND`):

```text
MAP W=FORWARD X=UTURN Y=LEFT Z=RIGHT
```

This is an example supplied at runtime, not a built-in letter assignment.
Pairs can appear in any order. Use four different uppercase A-Z letters and
exactly one each of `FORWARD`, `UTURN`, `LEFT` and `RIGHT`. The gateway itself
accepts at most 120 bytes (`S1_NET_MAP_TEXT_MAX`), but the
unchanged MQTT service accepts only 47 bytes (48-byte receive buffer including
NUL space). Keep PC payloads within 47 bytes. Spaces are accepted between pairs
and at the end. S1 uses S3's `nav_update_parse()` to
validate text syntax and copies a typed update into its dedicated map buffer.
S3 applies the stricter demo rule: exactly four different letters, one per
movement. Partial maps, repeated actions or NONE assignments may pass syntax
validation but are rejected by S3 without modifying the installed map. JSON,
lowercase, duplicate letters, embedded NULs and trailing newlines fail syntax
validation. The entire installation is atomic.

For example, from a PC with Mosquitto tools installed:

```sh
mosquitto_pub -h <broker-address> -t robot/cmd -m "MAP W=FORWARD X=UTURN Y=LEFT Z=RIGHT"
```

Send before starting a run. No letters have actions at boot. An accepted map
replaces all 26 entries, disabling every unselected letter, and increments
S3's `map_revision`. The map lives in RAM; resend it after a reboot. A Wi-Fi
reconnect does not itself erase it. A map is rejected while a decoded
instruction or an issued manoeuvre is pending, preserving that instruction.

The radio service remains the sole network owner. S1 copies the parsed update,
sequence and kernel receive timestamp into its dedicated two-entry map buffer.
The controller calls `s1_map_pump()` to submit at most one request per cycle.
A full S3 queue is retried until the original receive age exceeds one second;
stale/future requests and other submit failures are rejected. One admitted
request uses a one-second reply timeout, checked each controller cycle. Buffer overflow
keeps the existing drop-and-report policy; the PC must retry a dropped update.
Emergency stop uses its independent latch before ordinary command processing;
map queue congestion does not block the ordinary START/PAUSE/RESUME queue.

Queue admission is not acknowledgement of installation. The binary heartbeat
reports NONE, QUEUED, ACCEPTED or REJECTED plus the low eight bits of the map
sequence and applied revision. Only S3's matching accepted reply produces
ACCEPTED. Requests use Buddy 1's `S1_MAP_REQUEST_ID_FLAG | sequence`; there is
no second reserved `UINT32_MAX` adapter path. A rejected remote mapping
preserves the previous map and does not become a vehicle fault. Fault-bearing
replies still follow the controller's stop policy.

There is no separate MQTT mapping ACK topic: use the existing binary heartbeat
and S3 status during commissioning. MQTT publish success alone does not prove
installation. Legacy JSON heartbeats do not expose mapping acknowledgements.

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

Current cases: S1-U27 (complete demo map through the dedicated copied buffer),
S1-U26a-d (gateway syntax/ack/emergency path), S3-U38/39 (empty startup,
replacement, four actions, junction wait and repetition), INT-82 (controller
trigger), INT-83 (gateway queue retry), INT-84 (gateway rejection) and INT-85
(event-drain precedence). The former standalone S1-U23 mapping-command test is
retired with its duplicate transport; previous results remain in the logs.

The S1 ordinary command structure again contains only command ID and timestamp.
S3's public wire layouts are unchanged. Rebuild all owners with current headers.
Legacy `src/` programs remain bench tools. Cleanup evidence is appended to the
subsystem and integration test logs; earlier verification describes the original
pre-merge barcode change, not this working tree.

Historical equivalence scripts (`verify_barr_refactor.py` and
`verify_feedback.py`) compare against the former behaviour and are not
acceptance tests for this intentionally changed mission contract. Use
`verify_production_timing.py` for current S3 regressions.

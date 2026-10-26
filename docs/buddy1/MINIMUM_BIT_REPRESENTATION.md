# Buddy 1 — Minimum-Bit Data Representation (CSC12, PostTri1)

Owner: Buddy 1 (Wi-Fi / MQTT telemetry and commands). Branch `Buddy1-posttri`.
Code: `app_program/subsystem1/s1_codec.[ch]`, `s1_bits.h`, `s1_fault.h`, `s1_net.[ch]`, `s1_map.[ch]`.
Independent decoder: `testing/subsystem1/tools/s1_wire.py`. Evidence: `testing/subsystem1/logs/`.

## 1. Rules
Little-endian bytes; fields packed LSB-first; the first field starts at bit 0 of byte 0. Every field is read and written with an explicit `(value >> shift) & mask` through `s1_bits.h` (no C bit-fields, no struct overlay, no `memcpy` of native structs). Format field `1` = this layout; other values are rejected. A value that does not fit its width is never clamped or truncated: its validity bit is cleared and the event is counted (`telemetry_out_of_range`); encoder counts outside int32 reject the frame.

## 2. Telemetry (robot/telemetry): 232 bits = 29 bytes, every frame

| Field | Start bit | Bits | Range / unit | Notes |
|---|---|---|---|---|
| format | 0 | 2 | 1 | other = reject |
| seq | 2 | 12 | 0–4095, wraps | gap detection |
| state | 14 | 3 | 0–7 | vehicle state |
| validity | 17 | 6 | 1 bit per group | enc, line, hump, obstacle, IMU, time |
| fault | 23 | 8 | named bits (§5) | |
| time_ms | 31 | 32 | ms since boot | |
| speed L / R | 63 / 75 | 12 each | signed, ±2047 mm/s | |
| enc L / R | 87 / 119 | 32 each | signed int32 counts | |
| generation | 151 | 8 | 0–255 | |
| enc age | 159 | 10 | 0–1023 ms | |
| line | 169 | 3 | 0–7 | |
| line age | 172 | 10 | 0–999 ms | |
| hump | 182 | 12 | 0–4095 mm | |
| obstacle | 194 | 12 | 20–4000 mm | 0.1 cm steps |
| obstacle age | 206 | 10 | 0–1023 ms | |
| IMU age | 216 | 10 | 0–1023 ms | |
| reserved | 226 | 6 | 0 | must be 0 |

Heartbeat (robot/status): 8 bytes — format (2), estop (1), map result (2: NONE/QUEUED/ACCEPTED/REJECTED), reserved, uptime_ms (32), map seq (8), map revision (8), reserved (8).
Command (robot/cmd): exactly 1 byte — id in bits 0–2 (1 EMERGENCY_STOP, 2 START_RUN, 3 PAUSE, 4 RESUME), bits 3–7 must be 0.
Mapping update (robot/cmd): text `MAP A=FORWARD B=LEFT ...`, ≤120 bytes, ASCII, no NUL. First byte `M` (0x4D) can never be a valid command byte, so the two cannot be confused. Parsed with `nav_update_parse()`; accepted only when Buddy 3's reply says so.

## 3. Before / after (measured; `logs/minimum_bit_evidence.txt`)

| Item | Before (JSON v2) | After (binary) | Change |
|---|---|---|---|
| Telemetry payload, typical | 156 B | 29 B | −81 % |
| Telemetry payload, worst case | 225 B (239 B with all masks) | 29 B | fixed size |
| MQTT PUBLISH incl. headers | 175 B | 48 B | −73 % |
| Bytes/s at 10 Hz | 1750 | 480 | −73 % |
| Heartbeat | 38 B | 8 B | −79 % |
| Command EMERGENCY_STOP | 14 B | 1 B | −93 % |
| Telemetry queue (16 msgs, 8 B kernel overhead) | 1408 B | 592 B | −816 B |
| Code, s1_net.o text (Cortex-M0+) | 2144 B | 2785 B | **+641 B** |

The code grows by 641 B because the codec and the map gateway are new code; RAM and air-time fall. This is stated, not hidden.

## 4. Correctness evidence
- Golden vectors and boundary values: `test_codec_bin_host.c` (133 checks), including embedded zero bytes, unsupported formats, reserved bits, truncated and over-long buffers, multiple simultaneous faults.
- Independent decoder: `s1_wire.py` written from §2 only. `s1_crosscheck.py` compares 4000 frames: C bytes equal Python oracle bytes; 2220 of them exercised an out-of-range group.
- Network path: `test_net_bin_host.c` (68), mapping path `test_map_host.c` (38), on-target-emulated suite 62 checks incl. S1-U26 (map path), S1-U23–U25 (fault bits, two-core).
- Tribunal rows: (Buddy1+Buddy2 motion telemetry) = golden vectors + cross-check + sizes above; (EMERGENCY_STOP with command buffer full) = S1-U26d and the existing full-buffer E-STOP tests; (Buddy5+Buddy1 telemetry) = embedded zeros, bad versions, multi-fault, binary-vs-JSON size.

## 5. Shared fault bits (one kernel event flag, `tk_set_flg`/`tk_clr_flg` only)
MOTOR_STALL 0x01, ENCODER 0x02, IMU 0x04, NO_ECHO 0x08, WIFI_LOSS 0x10 (Buddy 1), ESTOP_LATCHED 0x20 (Buddy 1), LINE_SENSOR 0x40, BARCODE 0x80. `s1_net_fault_set/clear` refuse Buddy 1's bits (E_PAR). IMU and LINE_SENSOR are set from `vehicle_rt.c`; the other owners still have to call `s1_net_fault_set`.

## 6. Mapping update flow and atomicity
MQTT text → hook (`nav_update_parse`) → private buffer → `s1_map_pump` → `s3_submit` (**queued only**) → `s3_receive` reply matched by `request_id` → `s1_net_map_report` → heartbeat ack. A rejected map is reported, never an E-STOP. The ack is under the status mutex (hook blocks, controller waits ≤5 ms); an older seq never overwrites a newer one. Buddy 3 receives data only through S3's API, never Buddy 1's native structures.

## 7. Open items
Not run on hardware: NFR7 stop latency, NFR8 five-minute delivery, NFR9 CPU, on-target two-core tests. Full ARM link not completed here (SDK mismatch). Team has not signed off the layout or ranges (obstacle 12 bits vs proposed 9-bit cm is Buddy 5's call); instructor acceptance of binary for FR17 unconfirmed. `vehicle_rt.c` edits need the controller owner's approval. Map updates are not authenticated.
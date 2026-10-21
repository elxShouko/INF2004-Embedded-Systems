# Buddy 4 Data Representation After Tribunal 1

This note records the Buddy 4 portion of the minimum-bit change. It keeps the
single-owner IMU task, the public `imu_runtime_*` API, mutex-protected status
snapshots and the independent latched fault event. It does not change motor,
Mission Controller, telemetry or other buddy-owned interfaces.

## Implemented bounded fields

| Application data | Valid values | Required bits | Representation |
|---|---:|---:|---|
| IMU phase | 0 to 5 | 3 | `imu_core_status_t.phase` bit-field and packed word bits 0 to 2 |
| Motion class | 0 to 7 | 3 | `imu_core_status_t.motion` bit-field and packed word bits 3 to 5 |
| Heading valid | false or true | 1 | packed word bit 6 |
| Current hump height valid | false or true | 1 | packed word bit 7 |
| Session maximum valid | false or true | 1 | packed word bit 8 |
| Peak provisional | false or true | 1 | packed word bit 9 |
| Transition events | 9 independent events | 9 | packed word bits 10 to 18 |
| Height rejection reasons | 8 independent reasons | 8 | packed word bits 19 to 26 |
| Sample, calibration and private state flags | false or true | 1 each | private C bit-fields |

Bits 27 to 31 of the shared word are reserved and must be zero. The decoder
rejects a nonzero reserved bit, a phase above 5 or a motion class above 7.
`imu_packed.c` uses explicit masks and shifts for the inter-task representation;
it does not expose compiler-specific bit-field order as a queue or wire format.

## Measurements retained at 32 bits

Pitch, roll, heading, heading-derived turn rate, current hump height and session
maximum remain IEEE-754 32-bit values. A smaller representation is not yet
justified because the team has not agreed measured ranges, units and acceptable
quantisation error. Hump height also remains invalid unless calibrated,
fresh, straight, trusted wheel-motion evidence is present for the whole hump.

The 32-bit modulo-millisecond timestamp is retained because it is used for
wrap-safe freshness and duration checks. The saturating dropped-event counter
and motion-trust epoch also remain 32 bits because no lower lifetime bound has
been agreed.

## Size and RAM comparison

The host compiler and Arm build both enforce a 32-byte packed status. Arm GNU
15.3.1 size probes used the same RP2040 flags as the integrated Make build.

| Item | Host before | Host after | RP2040 before | RP2040 after |
|---|---:|---:|---:|---:|
| IMU event queue payload | 52 bytes | 32 bytes | 48 bytes | 32 bytes |
| Eight-message queue storage including two 32-bit kernel words per message | 480 bytes | 320 bytes | 448 bytes | 320 bytes |
| Public runtime status snapshot | 60 bytes | 40 bytes | 56 bytes | 40 bytes |
| IMU sample object | 60 bytes | 56 bytes | 60 bytes | 56 bytes |

The message buffer is allocated by the kernel from the requested size, so the
RP2040 request falls by 128 bytes. The private static runtime snapshot falls by
16 bytes in the linked image. The public API still returns named fields in
`imu_core_status_t`, so existing consumers do not decode the private queue
format.

## Concurrency and fault behaviour

- Only `imu_runtime_task` owns the sensor and processing state.
- The current packed snapshot is written and copied while holding the existing
  priority-inheritance mutex. Decoding occurs after the reader releases it.
- Queue entries are immutable copies and use non-blocking send/receive.
- A full ordinary event queue drops the newest ordinary event and increments
  the saturating counter; it cannot erase the current status snapshot.
- IMU, timing, driver, impact and unfinished-hump faults still latch the
  independent event flag before ordinary queue publication. Reading the flag
  does not clear it, and good samples cannot silently clear the core fault.
- Invalid or uncalibrated hump estimates keep their validity bits clear and are
  not published by the Vehicle Controller as measured height.

## Team agreement still required

The following are cross-buddy contracts and are deliberately not changed here:

1. Allocate Buddy 4's bit in the shared system fault map and agree whether the
   Vehicle Controller reports one IMU fault or multiple Buddy 4 cause bits.
2. Agree angle, turn-rate and hump-height ranges, units and precision before
   quantising those measurements or placing them in Buddy 1's binary telemetry.
3. Define when Buddy 2 and the Vehicle Controller may assert and revoke trusted
   straight, nonslip motion. The current default remains untrusted.
4. Measure end-to-end IMU-fault-to-stationary latency on hardware while
   preserving Buddy 2's exclusive motor ownership.

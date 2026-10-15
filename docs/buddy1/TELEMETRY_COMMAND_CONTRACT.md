# Buddy 1: real telemetry and command contract (PROPOSAL, not agreed)

Status: **proposal for team review.** Nothing in this file is agreed or implemented
unless it says so. Buddy 1 cannot decide the open items alone, because each one
belongs to another Buddy's data or to the (not yet existing) Vehicle/Mission
Controller. Until the table in section 3 is signed off, the telemetry source stays the
simulator (`S1_SIMULATE_TELEMETRY=1`) and nothing reads the command buffer except the
optional test monitor.

## 1. What Buddy 1 already provides (implemented, host-tested)

| ID (proposed) | Object | Meaning | Rules |
|---|---|---|---|
| IF-06 | `s1_net_telemetry_send(const s1_codec_frame_t *)` | one aggregated frame per 100 ms into a 16-frame message buffer | never blocks (`TMO_POL`); `E_TMOUT` when full; stale frames are dropped by the Wi-Fi task while offline |
| IF-07 | `s1_net_command_receive(&cmd, timeout)` | remote command with its arrival time | 8 deep; `EMERGENCY_STOP` evicts the oldest entry when full; other commands are dropped and counted |
| IF-08 | `s1_net_events_wait(pattern, &flags, timeout)` | `S1_NET_FLG_COMMAND`, `S1_NET_FLG_ESTOP`, `S1_NET_FLG_LINK_CHANGED` | `LINK_CHANGED` is raised once per change of the connected state, including entering backoff |
| status | `s1_net_get_stats()`, `s1_net_is_connected()` | one coherent snapshot of the counters and the connected state | brief mutex, never held across a network call |

Kernel objects used by Buddy 1 (for the TEAM.md budget, section 6): message buffers 2,
event flag 1, mutex 2 (`s1_net` status, `s1_sim` switch), cyclic handlers 2 (simulator,
template LED example), tasks 2 (simulator, template LED example). The Wi-Fi service task is
the template's. The self-test build adds two short-lived tasks and one flag.

## 2. Hard constraints already in force

* Only the Wi-Fi service task calls CYW43, lwIP or MQTT (TEAM.md section 3). No new network task is added.
* FR16: one frame every 100 +/- 10 ms. FR17 field names are fixed. NFR10: JSON at most 256 bytes
  (worst case today: 243 bytes, so any extra field must be paid for by shortening another).
* FR18/NFR7: a command is forwarded to its consumer within 50 ms. FR19: backoff 1, 2, 4, 8, 8 s.
* AGENTS.md: the emergency-stop path must not depend on an ordinary queue that can block, and
  must be reviewed and validated on hardware, with Buddy 2 keeping motor ownership.

## 3. Decisions the team has to make

"Today" is what the code or the other Buddies' headers say right now. "Proposed" is Buddy 1's
suggestion, not a decision. A row is closed only when its owner agrees and it is written into
TEAM.md section 5.

| # | Topic | Today | Proposed | Owner to agree |
|---|---|---|---|---|
| 1 | **Who builds the 100 ms frame** | Nobody. The simulator does. | Option A (recommended): a Buddy 1 aggregator task, driven by a 100 ms cyclic handler, reads each provider's copied-status getter and calls `s1_net_telemetry_send()`. Providers only need a thread-safe, non-blocking getter. Option B: the future Vehicle/Mission Controller builds the frame and Buddy 1 only transports it. | Team + instructor (it decides whether Buddy 1 may call other Buddies' public getters) |
| 2 | **Real sources** | `motion_api_status()` (S2), `s3_get_status()` and the line classifier (S3), `imu_runtime_get_status()` (S4), `s5_obstacle_profile_t` (S5). No S5 runtime getter exists on main yet. | Use only these public getters, called from the aggregator task, never from the Wi-Fi task. S5 must publish a copied-status getter. | Buddies 2, 3, 4, 5 |
| 3 | **Speed units and range** | `motor_speed_left/right` is `int16_t` with no unit defined. S2 reports `speed_mm_s[2]` as `int32_t`. | Signed millimetres per second per wheel, positive = forward, saturated to the `int16_t` range (+/-32767). | Buddy 2 (sign convention, which of measured or target speed to report) |
| 4 | **Encoder count** | Frame field is `uint32_t`, printed unsigned, so a reversing wheel would show about 4.29 billion. S2 exposes `position_um[2]` (signed 64-bit micrometres), not counts. | Signed 32-bit count per wheel, zero at boot, wraps at 2^31. S2 either exposes counts or the counts-per-micrometre constant so the aggregator can convert. Buddy 1 then changes the field to `int32_t`. | Buddy 2 |
| 5 | **Line status** | Frame: bit 0 left, bit 1 centre, bit 2 right (a placeholder written by Buddy 1). S3 produces a 2-bit black mask (bit 0 = GP2 left, bit 1 = GP4 right) and an observation enum (centred, correct left, correct right, absent). | Send the S3 observation enum (0 centred, 1 correct left, 2 correct right, 3 absent) as `line_status`. Drop the 3-bit placeholder. | Buddy 3 |
| 6 | **Hump peak** | Frame: `hump_peak_x10` (tenths, unit not stated). S4 reports `hump_peak_mm` as a float. | Millimetres, integer, converted by the aggregator; the JSON keeps one decimal only if the team wants centimetres. | Buddy 4 |
| 7 | **Obstacle distance** | Frame: `obstacle_distance_x10` (tenths of cm). S5 reports whole centimetres (`uint16_t`). | Send centimetres to match S5, with one decimal kept only if S5 can supply it. | Buddy 5 |
| 8 | **Validity and stale data** | The frame has no validity. A dead sensor looks like a good value. | Add a `valid_mask` byte to the frame (one bit each for motion, line, IMU/hump, obstacle). The aggregator clears a bit when the source reports invalid or its sample is older than 3 periods (300 ms, adjustable). The JSON prints `null` for an invalid field, which is also shorter than a number, so NFR10 holds. | Every provider (what "invalid" means for each) |
| 9 | **Remote-command consumer** | None. The test monitor `s1_cmd_echo` (patch, off by default) only prints. | The Vehicle/Mission Controller is the single reader of `s1_net_command_receive()`. Two readers would steal commands from each other, so the test monitor is removed when it exists. | Team (the controller owner) |
| 10 | **Emergency stop** | `EMERGENCY_STOP` is queued, evicts the oldest command if needed, and sets `S1_NET_FLG_ESTOP`. Nothing consumes it. | A small high-priority task waits only on `S1_NET_FLG_ESTOP` and calls `motion_api_estop()`. It never reads the ordinary command queue. This remote stop is a convenience path over Wi-Fi/MQTT, not the safety function: a local stop must exist independently. Needs Buddy 2's rules for which context may call `motion_api_estop()`, and a measured end-to-end time on hardware (NFR7: 50 ms). | Buddy 2 + team; hardware validation |
| 11 | **Simulation switch** | `S1_SIMULATE_TELEMETRY` defaults to 1; build with `-DS1_SIMULATE_TELEMETRY=0` to remove it. | The robot profile sets it to 0 once rows 1 to 8 are closed. A build-profile variable in `build_make` is a shared-file change and needs approval. | Team |
| 12 | **TEAM.md entries** | IF-06, IF-07, IF-08 are not recorded; the S1 budget row still shows the template example. | Add IF-06 to IF-08 and the S1 budget from section 1 once rows 1 to 10 are agreed. | Team (shared file) |

## 4. Shared files touched by Buddy 1 (need explicit approval)

| File | Change | Why |
|---|---|---|
| `lib/libwifi/sysdepend/pico_rp2040/cyw43_utk.c` | A refused first join request is now recovered through the bounded backoff; a refused retry re-arms the backoff; radio or interface failure stays permanent and is reported as not ready | feedback item 1 |
| `lib/liblwip/sysdepend/pico_rp2040/mqtt_utk.c/.h`, `net_backoff.h`, `lwipopts.h` | earlier work, already in the branch | MQTT client, backoff, ring-buffer sizes |

No other Buddy's implementation is modified.

## 5. Not yet verified

* Hardware: the status mutex test (S1-U14), the simulator switch test (S1-U15) and the first-join
  recovery have been run only on a PC. The two-core evidence needs `make TEST=1` on the Pico.
* The 5-minute delivery run (NFR8) and the CPU load during an outage (NFR9).
* Any real telemetry source and any command consumer (they do not exist yet).

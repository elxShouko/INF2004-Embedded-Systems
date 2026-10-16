# Team and subsystems

The tables are pre-filled with the **example** in `app_program/` (marked
*example*). Replace those rows with your own as you replace the example code.

Fill this in at the start of the project and keep it up to date. It is the
single place that says **who owns what**, so that five people can work in
one firmware image without clashing over pins, priorities, or kernel objects.

A **subsystem** is one student's part of the product, with its own folder.
It may contain several tasks and own several peripherals (see section 3).

Rule: **only the owner edits a subsystem's folders.** To change another
subsystem, ask its owner or open a pull request for them to review.

## 1. Members and subsystems

| # | Subsystem (what it does) | Student ID | GitHub username | Code | Tests |
|---|---|---|---|---|---|
| 1 | Wi-Fi/MQTT commands and telemetry; retained LED and simulator tests | | | `app_program/subsystem1/` | `testing/subsystem1/` |
| 2 | Motor/encoder motion control; retained producer example | | | `app_program/subsystem2/` | `testing/subsystem2/` |
| 3 | IR line sensing, junction detection and barcode navigation; retained example consumer for compatibility | 2500734 | | `app_program/subsystem3/` | `tests/member3_*.c`, `testing/subsystem3/` |
| 4 | IMU sensing, terrain/collision monitoring; retained monitor example | | | `app_program/subsystem4/` | `testing/subsystem4/` |
| 5 | Live ultrasonic/servo owner, scan/profile calculations; configurable certified-route planner; physical clearance evidence pending | | | `app_program/subsystem5/` | `testing/subsystem5/` |

Shared files (`app_program/app_main.c`, `app_program/app.h`, this file)
are changed only by agreement of the team. The exception is claiming an empty
row in this file (a pin, a peripheral, an interface ID), which anyone may do
(section 2).

## 2. Pins

One owner per pin. Claim a pin here **before** you wire or code it.

**Why:** two subsystems using the same pin is one of the most common and
hardest-to-find mistakes in a team project. The code builds without any
error, but the hardware misbehaves. Each subsystem can pass its own tests
and then fail once they are combined, and two outputs driving the same pin
against each other can damage it. Writing the claim down first means a clash
is spotted when someone edits this table, not hours later on the bench.

**No instructor check needed.** The team keeps this table itself. You do not
need the instructor, or the rest of the team, to check or approve a claim:
if the row is empty, fill it in and commit. If two people claim the same
pin, they agree between themselves who moves. Only the *reserved* rows are
fixed.

| Pin | Owner | Used for |
|---|---|---|
| GP23, GP24, GP25, GP29 | *reserved* | Pico W radio |
| GP0 | S2 | Right encoder A; UART/crash reporting must remain disabled |
| GP1 | S2 | Right encoder B; UART/crash reporting must remain disabled |
| GP2 | S3 | Left IR line sensor (user-confirmed assignment) |
| GP3 | | |
| GP4 | S3 | Right IR line sensor (user-confirmed assignment) |
| GP5 | | |
| GP6 | S5 | Ultrasonic ECHO (user-confirmed) |
| GP7 | S2 | Left encoder A; Week 6 assignment, wiring unverified |
| GP8 | S2 | Left motor M1A; PWM4A; Week 6 assignment, wiring unverified |
| GP9 | S2 | Left motor M1B; PWM4B; Week 6 assignment, wiring unverified |
| GP10 | S2 | Right motor M2A; PWM5A; Week 6 assignment, wiring unverified |
| GP11 | S2 | Right motor M2B; PWM5B; Week 6 assignment, wiring unverified |
| GP12 | | |
| GP13 | | |
| GP14 | | |
| GP15 | S5 (provisional; unverified) | Possible servo signal only; actual signal GPIO is not confirmed; do not configure |
| GP16 | S4 | I2C0 SDA, Week 6 design; physical wiring unverified |
| GP17 | S4 | I2C0 SCL, Week 6 design; physical wiring unverified |
| GP18 | | |
| GP19 | *example:* S1 | external LED (LED + 330 Ω to GND) |
| GP20 | | |
| GP21 | | |
| GP22 | | |
| GP26 | S5 | Ultrasonic TRIG (user-confirmed); use GPIO/SIO only (PWM5A aliases GP10 motor) |
| GP27 | S3 | Side IR barcode/junction sensor, digital DOUT |
| GP28 | S2 | Left encoder B; ADC2/VBAT jumper must not drive this pin |

Current observed Buddy 5 wiring is HC-SR04 VCC to 3V3, GND to GND, GP26 to
TRIG, and GP6 to ECHO. The module's operating suitability at 3V3, ECHO voltage,
and any level shifting are unverified. The servo model, signal GPIO, supply,
and wiring are unconfirmed; GP15 remains only an unverified candidate. GP26
must stay GPIO/SIO-only because it aliases PWM5A with Buddy 2's GP10 motor
output. These records do not authorize the explicitly configured Buddy 5 driver.

## 3. Peripherals

**Subsystems, tasks and peripherals are three different things:**

| Term | What it is | Example |
|---|---|---|
| **Subsystem** | one student's part of the product, one folder | "environment sensing", "motor control", "user interface" |
| **Task** | one thread of execution. A subsystem may have **as many tasks as it needs** | a sampling task and a filtering task, both in "environment sensing" |
| **Peripheral** | one hardware block on the RP2040 | I2C0, UART1, the ADC |

A driver is **not** a subsystem. It is code *inside* the subsystem that
needs that hardware: the "environment sensing" subsystem contains the I2C
sensor code, and owns I2C0. Write it as `drv_<name>.c` / `drv_<name>.h` in
that subsystem's folder, never under `device/`, and call it only from the
peripheral's owning task. The kernel's own drivers in `device/` (serial,
I2C, ADC) may be used instead; switching one on in `config/config_device.h`
changes shared configuration, so record it in the table below first.

**The rule: each peripheral is driven by exactly one task, its *owning
task*.** That task can be in any subsystem, and one subsystem may own
several peripherals (one owning task each, or one task owning all of them).
Every other task, in the same subsystem or another one, asks the owning task
through a kernel object (section 5) and never touches that peripheral itself.

*Why:* this port runs tasks on both cores at the same time. If two tasks
drive the same I2C or UART block at once, their transfers get mixed up, and
the drivers have no locking to stop it.

| Peripheral | Owner subsystem | Owning task | Pins |
|---|---|---|---|
| UART1 | | | |
| I2C0 | S4 | `imu_runtime_task` | GP16/GP17; hardware disabled until verified |
| I2C1 | | | |
| SPI0 | | | |
| SPI1 | | | |
| ADC | | | GP26–GP28 |
| PWM4A/B, PWM5A/B | S2 | motion owner (core 1) | GP8/9, GP10/11 |
| IO_BANK0 IRQ13 | Shared integration | dispatcher, core 1 only | S2 encoders and opt-in S5 ECHO GP6; cooperative subscriptions |
| PWM7B | S5 | s5_live_task, core 1; opt-in only | GP15 supported only after physical wiring/calibration confirmation; GP14 alias must be free |
| WiFi radio, TCP/IP, MQTT | *kernel* | `cyw43_utk_task` (built in) | GP23–25, GP29 |

Buddy 5 now has an explicit core-1 acquisition owner and PWM7B backend.
GP15 is still physically unconfirmed; configuration must attest wiring and
calibration before opening GPIO. TRIG GP26 remains SIO, never PWM5A.
Electrical suitability, pulse limits and timing remain hardware gates.

### WiFi and MQTT are special

The radio and the network stack belong to the kernel's built-in WiFi service
task, `cyw43_utk_task` (`lib/libwifi/sysdepend/pico_rp2040/`). **No student
task may call `cyw43_...`, lwIP or MQTT functions, not even from a
"WiFi subsystem".** The calls are not safe from any other task, on either core.

What students *can* do:

- **Read the connection state** from any task, which is always safe:
  ```c
  #include "cyw43_utk.h"
  T_CYW43_UTK_STATUS st;
  cyw43_utk_get_status(&st);   /* st.ready, st.link_status, st.ip_addr, ... */
  ```
- **Send or receive data over the network** by adding code that *runs
  inside the service task*. The MQTT client is in
  `lib/liblwip/sysdepend/pico_rp2040/mqtt_utk.c`. It publishes a heartbeat
  today. A "cloud / reporting" subsystem would add a kernel object
  (e.g. a message buffer) that application tasks fill. The MQTT code then
  reads it with `TMO_POL` (never blocking) and publishes it.
  Because this changes shared library code, **agree it with the team and
  the instructor first**, and record it as an interface in section 5.
  It needs the `NET=lwip MQTT=1` build (README section 3).

## 4. Task priorities

1 is the highest. 1–4 are used by the kernel (USB console 3, WiFi 4). Each subsystem gets its own band by default;
change it only by team agreement. Write down every task you create.

| Subsystem | Priority band | Tasks (name – priority – core) |
|---|---|---|
| 1 | 12–13 | *example:* led_task – 12 – any |
| 2 | 14–15 | *example:* producer_task – 14 – core 1 |
| 3 | 16–17 | retained consumer_task – 16 – core 0; s3_navigation_task – 17 – core 0 |
| 4 | 18–19 | retained monitor_task – 18 – any; imu_runtime_task – 19 – any |
| 5 | 20–21 | s5_live_task – 20 – core 1, static 4096-byte stack |

Shared vehicle task: priority 16, core 0, 10 ms service delay, static 8192-byte
stack. These are software scheduling choices, not measured timing guarantees.

Core: `TP_PRC1` = core 0, `TP_PRC2` = core 1, or "any".
Buddy 5 polls every 10 ms. Priority, stack headroom, callback latency and
servo/echo timing require target measurements; the delay is not a WCET bound.

## 5. Interfaces between subsystems

Every arrow between two subsystems is listed here. The **provider** creates
the kernel object and publishes its ID in its own `subsystemN.h`; the **user** only
sends to it or waits on it. Agree on the data format before coding.

| ID | From → To | Mechanism | Object ID name | Data / meaning | Agreed by |
|---|---|---|---|---|---|
| IF-01 | S2 → S3 | mailbox (owned by S3) | `mbx_samples` | `SAMPLE_MSG` in `subsystem2.h` | *example* |
| IF-02 | S3 → S2 | semaphore + fixed memory pool (owned by S2) | `sem_slots`, `mpf_samples` | receiver returns the block, then 1 slot | *example* |
| IF-03 | S3 → S4 | message buffer + event flag (owned by S4) | `mbf_summary`, `flg_events` | text line ≤ `S4_SUMMARY_MAX` bytes, then `FLG_BATCH` | *example* |
| IF-04 | S4 → S3 | shared data under a mutex (owned by S3) | `app_stats`, `mtx_stats` | `APP_STATS`, read only while locked | *example* |
| IF-05 | S4 → S1 | task control | `tid_led` | `tk_sus_tsk` / `tk_rsm_tsk` / `tk_chg_pri` | *example* |
| IF-06 | | | | | |

Buddy 5 now provides `s5_live.h`: immutable pre-start configuration, bounded
copied status (original capture timestamp/validity), dedicated fault/cancel
flag and core-1 shared dispatcher hooks. The central controller consumes
forward readiness and scan diagnostics, never direct motor outputs.
Configurable bypass execution and bounded moving line recovery are implemented;
actual movement remains blocked without calibrated geometry and scan-specific
verified swept corridors/original-line evidence.
See [Buddy 5 current status](docs/buddy5/OBSTACLE_SYSTEM.md#current-integrated-status-2026-10-16).

Each interface is tested by at least one integration test
(`testing/integration/TEST_LOG.md`).

Buddy 4 additionally provides `imu_runtime.h`: copied status/event reads,
a latched fault wait, and signed Buddy 2 motion feedback with explicit traction trust.
These are **provider proposals**, not agreed cross-subsystem interfaces;
the Phase 2 shared controller now consumes the fault latch and status,
requesting Buddy 2 emergency stop. End-to-end target latency is unverified.
Only Buddy 2 may actuate motors. See `testing/subsystem4/TEST_LOG.md` for test
scope and pending hardware/fault-to-stop acceptance checks.

## 6. Kernel object budget

Limits are set in `config/config.h`. If the team needs more, raise the
limit there. The numbers below are the example's usage.

| Object | Limit | S1 | S2 | S3 | S4 | S5 |
|---|---|---|---|---|---|---|
| Task | 32 | 1 | 1 | 2 | 2 | |
| Semaphore | 16 | | 1 | | | |
| Event flag | 16 | | | | 2 | |
| Mailbox | 12 | | | 1 | | |
| Mutex | 12 | | | 2 | 1 | |
| Message buffer | 12 | | | 2 | 2 | |
| Fixed memory pool | 12 | | 1 | | | |
| Variable memory pool | 12 | | | | | |
| Cyclic handler | 12 | 1 | | | | |
| Alarm handler | 12 | | | | 1 | |

The table above retains the professor example budget. Buddy 5 now adds one
task, one inheritance mutex and one event flag; the shared controller adds one
task and one inheritance mutex. Actual full-profile budgets and stack/heap
headroom require target review; no kernel object limits were changed.

(The kernel's USB and WiFi service tasks also use a few of these.)

## 7. Phase 2 shared integration (updated 2026-10-16)

Authoritative shared architecture: `app_program/vehicle.c/.h` (single-owner
mission policy), `vehicle_rt.c/.h` (micro T-Kernel/public-API adapter),
`gpio_bank.c/.h` (shared GPIO edge ownership). Tests and results:
[testing/integration/TEST_LOG.md](testing/integration/TEST_LOG.md).
This section supersedes older notes claiming no controller exists. Provider
proposals remain subject to their owners' review; no buddy implementation,
Makefile, kernel configuration or shared `app.h` was changed in Phase 2.

### Startup, arbitration and ownership

`usermain()` initializes all five subsystems and the controller before starting
any tasks. An initialization error prevents all task starts; no automatic
retry or teardown of another owner's partially initialized objects is attempted.
A subsystem start error posts Buddy 2 emergency stop and stops further starts;
S5's explicit `E_NOSPT` is recorded as missing readiness. The shared task helper
now deletes a dormant task and returns the original `tk_sta_tsk` error on failure.
Inherited producer/consumer/monitor/LED dependencies and TEST=1 calls remain.
Normal firmware disables the existing S1 synthetic telemetry producer before
starting tasks; TEST=1 preserves its demonstrations. TEST=1 is not a powered
motor test profile.

Only the vehicle task submits mission motion. S2 remains the only motor owner.
S3 classifies lines and decodes/maps barcodes; the controller selects continuous
signed wheel-speed requests or finite manoeuvres through S2's existing API.
STOP -> matching terminal -> ACK -> READY/standstill precedes new commands.
Queued intake is not acceptance: continuous refresh retains ID and awaits the
owner's matching ID/timestamp receipt. Finite commands never refresh. Cancelled
turns fail closed. Completed turns require correlated S3 MANOEUVRE_DONE and
fresh LINE_FOUND before following resumes. Obstacle proximity stops; no
unverified bypass or moving line search is substituted for S5 functionality.
Pause/obstacle stop preempts turn intake retries. Terminal ACK and recovery
waits have the configured transition deadline; recovery remains required across
PAUSE/RESUME. Provider clocks are refreshed after copied status reads, preventing
false future-sample faults when another core publishes during collection.
No finish-marker/mission-completion contract has been supplied.

There is no default calibration profile or `vehicle_runtime_setup()` caller.
The optional pre-start API copies one validated immutable mission/motion/S3
configuration and optional S5 acquisition/policy configuration; configuration updates, task restart and live fault reset are
unsupported (restart the firmware after addressing the cause). Explicit
calibration, power authorization, START/RESUME and fresh mandatory providers
are all required. Missing providers keep the mission blocked. Configuring a
future profile requires separate hardware approval; no Phase 2 code configures
motor GPIO during ordinary automatic startup.

The controller adds one task and one inheritance mutex, without increasing
kernel limits. Status uses copied snapshots and `TMO_POL`; no other task reads
its mutable owner context. Ordinary queue drains are bounded to eight S1
commands, four S3 outputs and four S4 transitions per cycle. Dedicated S1
persistent emergency latch/pulse, S4 and S5 fault latches are checked first; emergency is posted to S2 before ordinary
sensing/telemetry processing. The S4 status getter has a provider-defined 10 ms
bound. Poll period, priority, worst-case API/ISR execution, jitter, stop latency,
stack headroom and full network-profile object budget still need target review.

### Shared data and IRQ conventions

- S2 signed wheel speed: mm/s; signed wheel position: um; kernel operating-time
  timestamps: 64-bit us (10 ms kernel tick). No position-to-count reconstruction.
- S3 sample/nav timestamps: RP2040 raw 64-bit us. S4 timestamps: the same raw
  timer modulo 2^32 ms. S1 command rx timestamps: kernel modulo 2^32 ms.
  Freshness is checked in each source domain; future/stale/invalid samples
  cannot refresh motion. Modulo intervals are below half range.
- Barcode forward odometry is a relative mean of two positive wheel increments
  during continuous forward tracking. Signed arithmetic, bounded displacement,
  reversals/discontinuities and duplicate samples are checked. Half-um remainder
  is carried. Source age is preserved when mapping to raw time; a regressing
  mapped timestamp is rejected. It is not ground-truth distance or traction proof.
- IRQ13 is installed lazily only by an explicit S2 configuration on core 1.
  Installation refuses existing handlers or active/forced bank sources on either
  core. No pin mux is changed by the dispatcher. S2 reserves its four pins;
  explicitly configured S5 reserves GP6 through the same dispatcher.
  Other bank users must register with this owner, not replace its vector.
- Reserve/enable/release/callback operations are confined to core 1 with local
  interrupt exclusion; cross-core operations fail. Release quiesces callbacks
  before retiring static contexts. Owned bits use atomic SET/CLEAR and W1C;
  foreign pins/status are not overwritten. A bounded pending/levels/pending
  snapshot rejects changing/coalesced edges. Unknown enabled sources or lost
  edges disable the local bank NVIC and post S2 emergency, preventing an IRQ
  storm from starving the stop task. Recovery requires restart. The IRQ design
  requires SMP=1; SMP=0 compilation is checked but activation safely fails.

### Current verification and remaining gates

S1 threaded failures were traced to the host fake misinterpreting zero-valued
TWF_ANDW and a reader exhausting its poll budget before writer completion.
The fake and test scheduling are corrected; all 43 threaded checks pass.
The user-approved production telemetry v2 preserves signed int64 counts and
uint64 source time as decimal strings, with independent validity and source
ages. Legacy demonstration JSON remains unchanged; dashboard consumers must
use the documented v2 mapping. FR17 academic field-name acceptance still needs
confirmation. Production telemetry is rate-limited to at most 10 Hz.

S5 now starts an unconfigured core-1 owner without GPIO activation, supports
opt-in forward monitoring and stopped coarse/fine scanning, and exposes POL
copied status plus an independent fault latch. Forward readiness does not
depend on a complete scan or motor enable. Acquisition faults/cancellation
close owned outputs and latch a restart-required fault. Complete range scans
still cannot prove swept-path/rear/blind-sector clearance. The new configured
planner/controller implement mirrored eight-action routes and bounded forward
recovery, but require scan-specific verified free corridors and original-line
identity. No default certificate producer exists; physical coverage/environment
evidence is still required. The original range-only planner remains inhibited.

TEST=1 deliberately leaves the vehicle task dormant so it cannot consume the
professor demonstration test queues or inject production telemetry. Subsystem
self-tests remain present. This image is not a powered movement test profile.
Normal images contain no default mission/calibration setup caller.

Approved repository-local SDK 2.2.0 lives in ignored build/deps/pico-sdk.
The unchanged Make rules now link USB/Wi-Fi normal, TEST, IMU and lwIP/MQTT
profiles. Network-profile verification uses only the published example
credential header in ignored build/profiles; actual credentials/broker setup
and over-the-air operation remain unverified. No system-wide installation,
kernel/template/Makefile patch, flash or physical actuation was performed.

| Owner | Remaining gate |
|---|---|
| Buddy 1/team | Deploy v2 dashboard decoding and confirm FR17 representation; real Wi-Fi/MQTT/reconnection measurements |
| Buddy 2 | Confirm wiring, encoder levels/polarity, motor PID/geometry, stopping and SMP/IRQ latency |
| Buddy 3 | Physical IR/barcode calibration and sample-to-motion latency |
| Buddy 4 | IMU mounting/bus/calibration, traction trust and height validation |
| Buddy 5 | Servo pin/supply/endpoints, ultrasonic voltage/range/settling calibration; geometry/tracking/stopping calibration and a verified free-space/original-line evidence source |
| Shared | Explicit immutable mission profile/power authorization, measured resource budgets/WCET/stop latency and mission acceptance |

Exact source hashes, commands, results, evidence and artifact identities are in
testing/integration/TEST_LOG.md. Host fakes and successful linking do not prove
physical accuracy, target synchronization timing or autonomous completion.

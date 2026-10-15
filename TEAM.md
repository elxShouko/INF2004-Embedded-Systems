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
| 1 | *example:* LED heartbeat on GP19 | | | `app_program/subsystem1/` | `testing/subsystem1/` |
| 2 | *example:* Producer: makes a sample every 300 ms | | | `app_program/subsystem2/` | `testing/subsystem2/` |
| 3 | IR line sensing, junction detection and barcode navigation; retained example consumer for compatibility | 2500734 | | `app_program/subsystem3/` | `tests/member3_*.c`, `testing/subsystem3/` |
| 4 | *example:* Monitor: prints reports, pauses the LED | | | `app_program/subsystem4/` | `testing/subsystem4/` |
| 5 | Obstacle scan/profile calculations and avoidance planning (software-only; hardware integration pending) | | | `app_program/subsystem5/` | `testing/subsystem5/` |
| 6 | IMU sensing, calibration, terrain and collision events; retained monitor example | | | `app_program/subsystem4/` | `testing/subsystem4/` |
| 7 | | | | `app_program/subsystem5/` | `testing/subsystem5/` |

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
| GP0 | | (taken whenever the team builds with `CRASH_REPORT=1`) |
| GP1 | | (taken whenever the team builds with `CRASH_REPORT=1`) |
| GP2 | S3 | Left IR line sensor (user-confirmed assignment) |
| GP3 | | |
| GP4 | S3 | Right IR line sensor (user-confirmed assignment) |
| GP5 | | |
| GP6 | S5 | Ultrasonic ECHO (user-confirmed) |
| GP7 | | |
| GP8 | | |
| GP9 | | |
| GP10 | | |
| GP11 | | |
| GP12 | | |
| GP13 | | |
| GP14 | | |
| GP15 | S5 | Servo signal; Robo Pico GP15 servo header (user-confirmed) |
| GP16 | S4 | I2C0 SDA, Week 6 design; physical wiring unverified |
| GP17 | S4 | I2C0 SCL, Week 6 design; physical wiring unverified |
| GP18 | | |
| GP19 | *example:* S1 | external LED (LED + 330 Ω to GND) |
| GP20 | | |
| GP21 | | |
| GP22 | | |
| GP26 | S5 | Ultrasonic TRIG (user-confirmed); use GPIO/SIO only (PWM5A aliases GP10 motor) |
| GP27 | S3 | Side IR barcode/junction sensor, digital DOUT |
| GP28 | | (ADC2 capable) |

Buddy 5's user-confirmed wiring is GP26 for ultrasonic TRIG, GP6 for ECHO, and
GP15 for the servo signal. Sensor model, ECHO voltage/power suitability, and
servo electrical requirements remain unverified. GP26 must stay GPIO/SIO-only:
it aliases PWM5A with Buddy 2's GP10 motor output. These pin claims do not
enable the unimplemented Buddy 5 driver.

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
| PWM slices (list) | S5 (reserved; task TBD) | TBD — hardware integration pending | PWM7B (GP15 servo); do not initialize until approved |
| WiFi radio, TCP/IP, MQTT | *kernel* | `cyw43_utk_task` (built in) | GP23–25, GP29 |

Buddy 5's servo PWM resource is reserved as slice 7 channel B (GP15); its
owning task remains TBD. Ultrasonic GPIO acquisition and all drivers remain
unimplemented. Do not initialize or drive these peripherals until the task,
electrical checks, and startup/fault behavior are reviewed.

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
| 5 | 20–21 | |

Core: `TP_PRC1` = core 0, `TP_PRC2` = core 1, or "any".
The Buddy 5 priority band is available, not an assignment of task names,
periods, affinity, or startup behavior; these remain `TBD`.

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

No Buddy 5 interface is agreed or registered yet. Stop/fault, motion,
line-reacquisition, and optional telemetry contracts are placeholders only;
record agreed provider, consumer, mechanism, object, data/lifetime, errors,
and acknowledgement semantics here before implementing cross-subsystem calls.
See [Buddy 5 unresolved integration placeholders](docs/buddy5/OBSTACLE_SYSTEM.md#unresolved-integration-placeholders).

Each interface is tested by at least one integration test
(`testing/integration/TEST_LOG.md`).

Buddy 4 additionally provides `imu_runtime.h`: copied status/event reads,
a latched fault wait, and a proposed calibrated forward-speed input.
These are **provider proposals**, not agreed cross-subsystem interfaces;
no Vehicle/Mission Controller exists here. Fault flags do not stop motors.
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

Buddy 5 currently allocates no kernel objects; its future demand remains
`TBD` and must be reviewed before task or interface implementation.

(The kernel's USB and WiFi service tasks also use a few of these.)

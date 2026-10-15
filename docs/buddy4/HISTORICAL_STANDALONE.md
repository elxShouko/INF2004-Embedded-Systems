# Buddy 4 — IMU-Based Motion and Terrain Monitoring (historical)

> Historical standalone implementation guide. Current production firmware is
> in `app_program/subsystem4/`; the 113-test suite described here targets the
> older `src/imu/` code. All current tests are under `testing/subsystem4/`.
> See the [current test registry](../../testing/subsystem4/TEST_LOG.md) for
> current source paths, commands, test scope and results. Claims and commands
> below describe the historical standalone revision, not the current build.

**Owner:** Eddy Teo Hon Kai (2501284) · **Group:** CSC12 · **Branch:** `buddy4-imu`
**Hardware:** Raspberry Pi Pico W + Robo Pico, LSM303DLHC on Grove 4 (I2C0, SDA GP16, SCL GP17)
**Software:** C11, Raspberry Pi Pico C SDK, µT-Kernel 3.0, Barr-C style

> **Verification status, stated plainly.**
> - **Verified here:** the code compiles with zero warnings against Pico SDK 2.1.1 using arm-none-eabi-gcc 14.2.1. `imu_rtos.c` compiles against the real µT-Kernel 3.0 headers from TRON Forum `mtk3_bsp`, branch `pico_rp2040`. All 113 host tests pass (2,939 assertions), and they also pass under AddressSanitizer + UBSan. clang-tidy reports no findings.
> - **Not verified:** nothing has run on a physical Pico W, LSM303DLHC or robot car. Every threshold in `imu_config.h` is an *unvalidated development default*. Accuracy figures below come from a physics simulator, not from the track.

---

## Contents

1. [Part 1 — Requirements audit](#part-1--requirements-audit)
2. [Part 2 — Hardware and design limitations](#part-2--hardware-and-design-limitations)
3. [Part 3 — Architecture](#part-3--architecture)
4. [Part 4 — Code (file list)](#part-4--code)
5. [Part 5 — Build and installation](#part-5--build-and-installation)
6. [Part 6 — Testing](#part-6--testing)
7. [Part 7 — Hardware calibration and threshold tuning](#part-7--hardware-calibration-and-threshold-tuning)
8. [Part 8 — Team integration](#part-8--team-integration)
9. [Part 9 — Compliance review](#part-9--compliance-review)

---

## Part 1 — Requirements audit

Sources: Project Write-up (PW), Student Briefing (SB), Week 6 Design Review (DR).

| # | Requirement (source) | Implementation (file → function) | Verified by test |
|---|---|---|---|
| R1 | FR7: acquire and process motion data | `imu_lsm303dlhc.c` → `lsm303_read_accel/mag`; `imu_terrain.c` → `terrain_acquire`, `terrain_process` | `t_fifo_read`, `t_mag_order_and_scale`, `bb04_stationary` |
| R2 | FR7: detect tilt | `imu_processing.c` → `imu_proc_step` (pitch/roll/tilt), `proc_tilt_step` | `t_pitch_roll_sign`, `t_tilt_detector`, `bb15_tilt` |
| R3 | FR7: motion state | `imu_proc_classify` | `t_classifier`, `t_classifier_no_flapping`, `bb06`, `bb07`, `bb08` |
| R4 | FR8: detect hump events | `imu_hump.c` → `imu_hump_step` (ASCENT→PEAK→DESCENT) | `t_canonical`, `bb09`, `bb10`, `bb22`, `bb23` |
| R5 | FR8: measurable hump-peak value | `imu_hump_meas_t.peak_pitch_deg` (measured) + `imu_hump_estimate_height` (estimate) | `t_height_estimates`, `bb09`, `bb10`, `bb24` |
| R6 | FR8 / PW mission 5: retain the highest peak of the run | `hump_finish` → `stats.max`, updated only on a VALID and strictly higher hump | `t_multiple_retain_max`, `bb11_multiple_humps` |
| R7 | FR9: turning events | `proc_turn_step` (START/END/HEADING_CHANGE) | `t_turn_left_right`, `bb08_turns`, `bb05_mounting` |
| R8 | FR9: sudden impact / collision | `imu_proc_hf_accel` (full-rate high-pass on every FIFO sample) | `t_impact_detector`, `t_impact_cooldown`, `bb13`, `bb14` |
| R9 | SB: report turn rate | `turn_rate_dps` (heading-derived, flagged) | `t_turn_rate_wraparound` |
| R10 | SB: calibrate the IMU | `imu_calibration.c` (stationary accel + hard/soft-iron mag) | `t_accel_cal_*`, `t_mag_cal`, `bb03`, `bb25` |
| R11 | NFR5: reliable hump detection / stable classification / consistent peak | Hysteresis, minimum durations, re-arm, crest timeout, debounce | `t_spike_rejected`, `t_no_double_count`, `bb12_noise_no_false_humps`, `bb24_speed_repeatability` |
| R12 | NFR11: C + Pico SDK + µT-Kernel | `imu_hal_pico.c` (hardware_i2c), `imu_rtos.c` (tk_* calls) | Firmware build; µT-Kernel header compile check |
| R13 | NFR12: Barr C + repo structure | `include/imu/*`, `src/imu/*` (same layout as Buddy 5) | clang-tidy; strict `-W` flags; deviations listed in Part 9 |
| R14 | NFR13: clear interfaces | `imu_events.h` (MC), `imu_rtos.h` (API), `imu_hal.h` (port) | Black-box suite uses only public APIs |
| R15 | NFR14: prudent CPU/RAM | Static allocation, no heap, FIFO batch reads, bounded queues | `p15_memory_budget`, `-fstack-usage` |
| R16 | NFR15: one coordinated vehicle | `terrain_event_q`, direct event-flag notification, telemetry snapshot | `r04`, `r05` (fake kernel) |
| R17 | DR §2: GP16/GP17 on I2C0 | `IMU_I2C_SDA_GPIO/SCL_GPIO`, `IMU_I2C_INSTANCE` | `imu_config.h` (checked against the RP2040 pin-function table) |
| R18 | DR §3: periodic task, Medium–High priority | `imu_rtos.c` → cyclic handler + task, priority 9 | `r03_objects_and_attributes` |
| R19 | DR §4: `terrain_event_q` with HUMP_DETECTED, HUMP_PEAK, TILT_EVENT, TURNING_EVENT, IMPACT/COLLISION, IMU_FAULT, plus a collision notification | `imu_events.h` (`imu_event_t`), `rtos_send`, `rtos_notify` | `t_overflow_policy`, `bb20_queue_overflow`, `r05` |
| R20 | DR §5/§6.4: statechart | `imu_terrain.c` state machine (mapping in Part 3) | `bb01`, `bb13`, `bb16`, `bb17`, `bb18`, `bb27` |
| R21 | DR §6.4: unresolved faults never return silently | Recovery needs verification; collision needs MC acknowledgement + settled conditions | `bb13`, `bb17_persistent_fault`, `bb18` (frozen) |
| R22 | DR §7 A-B4-1 / A-B4-2 | Bring-up logger + `analyse_log.py` + replay tool | Part 7 procedure |
| R23 | DR §8.2: never drive the motors | No motor API is referenced (verify: `grep -r motor src/imu`) | Code review |
| R24 | DR §8.2: thresholds configurable, not fixed | `imu_params_t` (run-time) + `#ifndef` macros | `t_params_validate`, `p08_parameter_fuzz` |
| R25 | FR17: `hump_peak` in telemetry | `imu_get_telemetry`, `imu_terrain_telemetry_json` | `bb28_telemetry_json` |
| R26 | SB deliverable: terrain analysis report | Template and procedure in Part 7 | — (needs hardware data) |

### Conflicts, gaps and limitations found while auditing

| ID | Finding | Resolution |
|---|---|---|
| G1 | The PW asks to *"estimate hump height"*. The LSM303DLHC cannot measure height directly. | Report the measured peak pitch (primary) plus a height estimate with explicit method and confidence. See Part 2. |
| G2 | Your repo has **no µT-Kernel yet**, and no queue definitions or Mission Controller. | `imu_rtos.c` targets the standard µT-Kernel 3.0 API (checked against TRON's RP2040 port). Queue ID, flag ID and priorities are configuration inputs, marked **CONFIRM**. |
| G3 | TRON `mtk3_bsp` `pico_rp2040` configures **I2C0 on GP8/GP9**, which are Buddy 2's left motor pins (DR §2). Its I²C device driver is **enabled by default** (`DEVCNF_USE_IIC 1` in `config/config_device.h` → `dev_init_i2c(0)` at boot). | Buddy 4 uses Pico SDK `hardware_i2c` on GP16/GP17. If the course template is based on this BSP, set `DEVCNF_USE_IIC` to **0**; otherwise it reconfigures the motor pins. |
| G4 | That port has `CNF_MAX_MBFID = 4`, but DR §4 lists **8 queues**. `CNF_MAX_MTXID = 4` and `CNF_MAX_CYCID = 4` are also tight. | Raise these in the kernel's `config.h` (Part 8). |
| G5 | Its `tk/syslib.h` typedefs `size_t`, which conflicts with newlib / Pico SDK headers. | Build with `-DPROHIBIT_DEF_SIZE_T` (already set in CMake). |
| G6 | That port's tick is 10 ms. | The sampling period must be a multiple of the tick. The default of 20 ms is. |
| G7 | DR §6.4 updates the maximum *at the peak*. A false candidate could then corrupt the run maximum. | The comparison is computed at HUMP_PEAK (provisional value, `IMU_EVT_HUMP_PEAK`) but **committed** only when the traversal completes and is validated (`IMU_EVT_HUMP_COMPLETED`, `b_new_max`). |
| G8 | DR §4 lists no "hump complete/rejected" message, but the MC needs to know when a hump ends. | Added `IMU_EVT_HUMP_COMPLETED` and `IMU_EVT_HUMP_REJECTED`. |
| G9 | Telemetry field `hump_peak` has no unit in FR17. | Default is centi-degrees of peak pitch (measured). Height in mm is also provided (`hump_peak_mm`). **Confirm with Buddy 1.** |

---

## Part 2 — Hardware and design limitations

### What the LSM303DLHC actually measures

| Quantity | Measured? | How Buddy 4 obtains it | Main limitation |
|---|---|---|---|
| Specific force (gravity + linear acceleration), 3 axes | **Yes** (±4 g, 12-bit, 2 mg/LSB, 200 Hz, FIFO) | Direct | Gravity and acceleration cannot be separated without other information |
| Magnetic field, 3 axes | **Yes** (±1.9 G, 75 Hz) | Direct | Motors, wiring and steel distort it |
| Pitch / roll / tilt | Derived | From the gravity direction | Wrong during acceleration: error ≈ asin(a/g), e.g. 0.06 g → 3.4° |
| Heading | Derived | Tilt-compensated magnetic heading (vector method) | Magnetic disturbance; needs hard/soft-iron calibration |
| Turn rate | **Estimated** | d(heading)/dt over a 200 ms window | **Not a gyroscope**: delayed ~100 ms, band-limited, invalid when the field is disturbed |
| Angular velocity | **No** | — | No gyroscope on this chip; none is invented |
| Speed / distance | **No** | Optional Buddy 2 encoder hint | Integrating acceleration drifts within seconds |
| Stationary vs constant speed | **No** (physically indistinguishable) | Encoder hint gives HIGH confidence. Without it, a "no vibration" heuristic gives LOW confidence. | Class `STEADY_MOTION` is used instead of guessing |
| Hump height | **No** | Estimated (below) | Needs the wheelbase and assumptions about hump shape |
| Impact | Yes (short high-pass spikes) | Every FIFO sample at 200 Hz | Pulses under ~5 ms are attenuated; anything above ±4 g saturates (still detected) |

### Hump peak: measured metric versus physical height

**Measured (primary, used to rank humps):** the peak nose-up pitch θ_peak, relative to the level reference calibrated at power-up. For a given vehicle and hump it repeats, and it increases monotonically with hump height for humps no longer than the wheelbase. It is what the IMU actually sees.

**Estimated (secondary):** `height_est_mm`, with `height_conf` and `height_method`.

1. **GEOMETRIC (default)**, `h = L · sin(θ_peak)`, then `h ← gain·h + offset`.
   A rigid car with front and rear contact points L apart, rear on the floor and front at height h, has pitch θ with sin θ = h/L. That holds while the front wheels are at the crest and the rear wheels are still on level ground, which is true if the hump length is ≤ L.
   - It is wrong for humps much longer than L, where pitch only reflects the ramp slope. When encoder speed is available, the length is measured and confidence is lowered for humps longer than 1.5 L.
   - It depends on wheel radius and contact geometry, a caster wheel, suspension, and pitch errors from acceleration.
   - It is attenuated at higher speed by the attitude low-pass filter (see below).
2. **INTEGRATED** (`height_method = IMU_HEIGHT_METHOD_INTEGRATED`), `h = max ∫ v·sin θ dt / k_pos`.
   This needs a fresh Buddy 2 speed hint during the whole ascent; without it, the code falls back to GEOMETRIC. `k_pos` is the IMU's position along the wheelbase.
3. **Empirical correction:** `hump_height_gain` and `hump_height_offset_mm` must be fitted from at least 3 humps of known height (Part 7). Until `b_hump_height_calibrated = true`, every estimate is reported with **IMU_CONF_LOW**.

**Data needed for a defensible absolute height:** the measured wheelbase (`IMU_CFG_WHEELBASE_MM` is a **placeholder of 140 mm**), hump profiles (length and shape), encoder speed, and repeated passes over humps of known height.

**Simulated accuracy** (physics model, L = 140 mm, raised-cosine hump 100 mm long, 0.25 m/s, vibration 0.02 g). This is evidence for the algorithm, **not** for the hardware.

| Hump height | True θ_peak | Measured θ_peak | Estimated h |
|---|---|---|---|
| 12 mm | 4.92° | 5.10° | 12.4 mm |
| 30 mm | 12.37° | 12.29° | 29.8 mm |
| 20 mm at 0.12 / 0.20 / 0.30 / 0.45 m/s | 8.21° | 8.19 / 7.86 / 7.44 / 6.80° | 19.9 / 19.2 / 18.1 / 16.6 mm |

The 17 % drop at higher speed comes from the 8 Hz attitude low-pass filter shortening a fast pitch pulse. Two ways to deal with it:
- Calibrate `gain` at the mission's operating speed.
- Raise `attitude_lpf_hz`, which also raises noise.

Comparing humps is unaffected as long as all humps are crossed at similar speed, which the PID-controlled Buddy 2 should provide.

### Other limitations

- **Acceleration versus hump (without encoder hint):** pulling away from standstill at 0.06 g produces ~3.4° of apparent pitch.
  - Such a candidate is **never** counted as a hump: it lacks the nose-down phase, so it is rejected after `hump_crest_max_ms` with `REJ_NO_DESCENT` (test `bb07b`).
  - The MC may still see a `HUMP_DETECTED` followed by a `HUMP_REJECTED`.
  - Accelerating, then braking within ~1.5 s, could in theory mimic a hump. The Buddy 2 acceleration hint removes this (`bb07`).
- **Consecutive humps** must be separated by ≥ `hump_rearm_level_ms` of level ground (150 ms ≈ 4 cm at 0.25 m/s).
- **Impacts during FAULT/SAFE** are not reported again. The vehicle is already in the safe state, and repeated hits keep the "settled" condition false.
- **I²C read cost:** each FIFO sample is read separately (6 bytes), because burst reading across the FIFO is not documented for the DLHC. That is ~1 ms of polled bus time per cycle (Part 3).

---

## Part 3 — Architecture

### Layers and files

```
include/imu/                          src/imu/
 imu_types.h        common types, enums (no deps)
 lsm303dlhc_regs.h  register map (datasheet + reference drivers)
 imu_config.h       pins, defaults (UNVALIDATED), imu_params_t    imu_params.c
 imu_hal.h          I2C + time port interface                    imu_hal_pico.c   (Pico SDK)
 imu_lsm303dlhc.h   device driver                                 imu_lsm303dlhc.c
 imu_math.h         vectors, rotation, wrap, IIR                  imu_math.c
 imu_calibration.h  stationary accel + mag hard/soft iron         imu_calibration.c
 imu_processing.h   attitude, heading, turn, tilt, impact, class  imu_processing.c
 imu_hump.h         hump FSM + measurement                        imu_hump.c
 imu_events.h       terrain_event_q message + bounded dispatcher  imu_events.c
 imu_terrain.h      subsystem state machine (RTOS-agnostic)       imu_terrain.c
 imu_rtos.h         µT-Kernel task + thread-safe public API       imu_rtos.c
tools/buddy4_bringup/  bring-up / calibration logger (own main), analyse_log.py
tests/imu/      historical simulator, 113 tests, fake kernel, replay tool (retired)
```

Separation of concerns:
- The driver knows registers, not motion.
- Processing knows physics, not I²C.
- `imu_terrain` owns the state machine and is plain C (unit-testable).
- `imu_rtos` is the only file that uses µT-Kernel.
- `imu_hal_pico` is the only file that uses the Pico SDK.

### Data flow per 20 ms cycle

```
cyclic handler ──tk_wup_tsk──▶ IMU task
   │ 1. take commands + encoder hint   (mutex, µs)
   │ 2. imu_terrain_step(now):
   │      FIFO_SRC_REG_A → n samples (≈4 at 200 Hz) → each: impact detector (high-pass, full rate)
   │      cycle average → level → LPF 8 Hz → pitch/roll/tilt (+hint compensation)
   │      magnetometer → hard/soft iron → level → tilt-comp heading → LPF → unwrap → 200 ms rate
   │      tilt detector │ turn detector │ hump FSM │ motion classifier
   │      events → dispatcher → tk_snd_mbf(terrain_event_q, TMO_POL)   (+ tk_set_flg if critical)
   │ 3. publish status snapshot (mutex, ~0.5 kB copy)
   │ 4. overrun check (tk_can_wup)
```

### Subsystem state machine (DR §6.4) and how it is implemented

| DR §6.4 state | Implementation | Entry / exit conditions |
|---|---|---|
| INITIALISATION | `IMU_STATE_INITIALISATION` | Retries `lsm303_init`: every 200 ms, then exponential back-off up to 2 s. At 5 failures it sends IMU_FAULT (FAULT_RAISED) once and **stays** here (DR dashed arrow) |
| CALIBRATION | `IMU_STATE_CALIBRATION` | 100 cycles of still data. On failure (motion, range): STATUS CAL_RETRY, retry after 500 ms; after 5 failures, IMU_FAULT CAL_FAILED. On success: STATUS READY → MONITORING |
| MONITORING | `IMU_STATE_MONITORING` | Normal sampling |
| HUMP ASCENT / PEAK / DESCENT | `IMU_STATE_HUMP_*`, mirrored from `imu_hump.c` | See the hump algorithm below |
| TILT EVENT → EVENT REPORTING | Transient `IMU_STATE_TILT_EVENT` (logged in the trace, not persistent) | Reported **without interrupting** monitoring or hump processing |
| TURN EVENT → EVENT REPORTING | Transient `IMU_STATE_TURN_EVENT` | Same as tilt |
| IMPACT EVENT → FAULT HANDLING → FAULT/SAFE | `IMU_STATE_IMPACT_EVENT` → `FAULT_HANDLING` → `FAULT_SAFE` (fault = COLLISION) | CRITICAL impact: event + direct flag. MINOR impact: reported, monitoring continues (configurable) |
| SENSOR FAULT → FAULT/SAFE | `IMU_STATE_SENSOR_FAULT` → `FAULT_SAFE` | 5 consecutive I²C failures, 5 implausible samples, frozen output (50 cycles), or no data for 200 ms |
| "Recovery and conditions normal" | `IMU_STATE_RECOVERY` → MONITORING | **Sensor fault:** bus-clear + re-init + verify 10 plausible, changing samples, then IMU_FAULT RECOVERED. **Collision:** MC `CLEAR_FAULT` **and** level and steady for 500 ms, then STATUS RESUMED |
| Unresolved fault | Stays `FAULT_SAFE` | After 10 failed recoveries, IMU_FAULT PERSISTENT (once). Keeps retrying every 2 s and never resumes without verification |
| "Subsystem reset" | `IMU_CMD_RESET` → INITIALISATION | From any state |
| (added) STOPPED | `IMU_CMD_STOP` / `imu_rtos_stop()` | Magnetometer asleep, accelerometer powered down, bus released |

**Why tilt and turn are transient instead of exclusive states:** the DR diagram draws them as branches out of MONITORING, but a hump can be crossed during a gentle turn. Making them exclusive would freeze hump processing while a turn event is "being reported". So the detectors run in parallel every cycle (an orthogonal region in UML terms). The transient state is recorded in the trace, which keeps the reporting step visible and testable. This matches the system statechart (§5), where Terrain Monitoring's REPORT_EVENT returns to monitoring immediately.

### Hump detection algorithm (mathematical basis)

Input: θ(t), the filtered pitch relative to calibrated level, corrected by the encoder acceleration when available (`f_x ← f_x − a_enc/g`).

1. **Armed** when |θ| < 1.5° for ≥ 150 ms and ≥ 300 ms have passed since the last completed hump. This prevents double counting.
2. **ASCENT** when θ > 3.0° for ≥ 60 ms (`HUMP_DETECTED`). The candidate maximum is tracked from the first sample, so fast humps are not under-read. A short spike fails the duration test.
3. **PEAK** when θ ≤ θ_max − 1.0° (`HUMP_PEAK`, provisional). The 1° drop is hysteresis against noise at the crest.
4. **DESCENT** when θ < −2.5° for ≥ 40 ms. This nose-down phase (front wheels descend, or rear wheels climb) is the signature that separates a hump from a ramp or an acceleration artefact.
5. **Complete** when θ > −1.5° for ≥ 100 ms. Then validate: θ_max ≥ 3.5°, θ_min ≤ −2.0°, and duration within 150 ms–4 s. `HUMP_COMPLETED` → update last; update max if θ_max > max + 0.001°.
6. **Rejections:**
   - ascent longer than 2 s → `REJ_SLOPE`
   - no nose-down phase within 1.5 s of the crest (1.5 L with encoders) → `REJ_NO_DESCENT`
   - over 4 s → `REJ_TOO_LONG`
   - critical impact or fault → `REJ_ABORTED`
   - A rejected candidate does not block the next hump: no gap penalty, and a fresh climb out of the crest restarts immediately.

Speed independence comes from time windows wide enough for 0.1–0.5 m/s (tests sweep 0.25×–3× time scale) and from the optional distance-based crest limit.

### Other algorithms

- **Tilt:** total tilt > 15° for ≥ 300 ms → START. Below 12° (3° hysteresis) for 300 ms → END. ≥ 35° → CRITICAL immediately (also on the direct flag). During a hump, only roll is used, unless tilt is already critical.
- **Turning:** |rate| > 25 °/s for ≥ 200 ms → START (rate < 0 = LEFT, because compass heading decreases on a left turn). |rate| < 12 °/s, a reversal, or an invalid heading for 200 ms → END, with the total heading change.
  - A slow drift ≥ 20° produces HEADING_CHANGE.
  - The rate is invalid when |B| differs > 25 % from the calibrated reference (DISTURBED) or the magnetometer overflows.
  - A single heading jump > 45° is rejected as an outlier.
- **Impact:** every FIFO sample → levelled → high-pass (x − LPF₂Hz(x)).
  - Pulse when |hp| > 1.2 g (×1.5 while in a hump); it ends below 70 %.
  - A pulse lasting > 60 ms is not an impact (sustained manoeuvre).
  - CRITICAL if peak ≥ 3 g, or peak ≥ 2 g and horizontal share ≥ 60 % (collision-like). Hump edges are mostly vertical, so they stay MINOR.
  - 1 s cool-down; suppressed pulses are counted, and an escalation from MINOR to CRITICAL is always reported.
- **Motion classification:** priority order is IMPACT (held 500 ms) > HUMP phase > TURN > ACCEL/DECEL (0.06 g enter / 0.03 g exit) > STATIONARY/STEADY.
  - The last two classes are debounced 150 ms.
  - Encoder hint present → STATIONARY/STEADY with HIGH confidence. Without it → "no vibration" heuristic, LOW confidence.

### µT-Kernel task design and processing budget

| Item | Value | Reason |
|---|---|---|
| Trigger | Cyclic handler (`tk_cre_cyc`, 20 ms) → `tk_wup_tsk` | Drift-free period. `tk_dly_tsk` would add the execution time to every period |
| Task wait | `tk_slp_tsk(3 × period)` | Bounded wait. A missing handler is detected (`missed_wakeups`) and the task never blocks forever |
| Priority | 9 (CONFIRM) | Medium–High: below Motion (4), Line/Obstacle/MC (6–7), above Wi-Fi (14) |
| Stack | 2048 B static (TA_USERBUF), 8-byte aligned | Deepest measured frame chain ≈ 216 + 248 + 168 B, plus libm and ROM float ≈ 1 kB → ~2× margin |
| Queue | `terrain_event_q` message buffer, 16 × 36 B, static (TA_USERBUF), `TMO_POL` | Bounded and non-blocking (overflow policy in `imu_events.h`) |
| Mutex | TA_TPRI \| TA_INHERIT, 10 ms bounded wait | Priority inheritance; the sensor is never accessed while holding it |
| Heap | none | `TA_USERBUF` everywhere; `memset`/copies only |
| Overrun | `tk_can_wup(TSK_SELF)` after each cycle | Counts and discards queued wake-ups (no catch-up bursts) |

**Estimated budget per cycle (RP2040 @ 125 MHz; estimate — measure with `imu_rtos_get_stats().max_cycle_us`):**

| Work | Estimate |
|---|---|
| I²C @ 400 kHz: FIFO_SRC (≈90 µs) + 4 × 6-byte reads (≈4 × 225 µs) + magnetometer 6 bytes (≈225 µs) | ≈ 1.2 ms (polled, preemptible) |
| Float math, soft-float via RP2040 ROM: 4 × HF path + main step (3 atan2f, ~5 sqrtf, rotations, filters) | ≈ 0.15–0.3 ms |
| Events + snapshot copy | < 0.05 ms |
| **Total** | **≈ 1.5 ms per 20 ms ≈ 7 % CPU** |

The I²C transfers busy-poll inside the IMU task. Higher-priority tasks (Motion, Line, MC) preempt them at any time, so they are not delayed. Only lower-priority tasks (Wi-Fi) are. Reducing the ODR to 100 Hz halves the bus time at the cost of impact bandwidth.

Code size (arm-none-eabi-gcc, -Os): ≈ 20 kB of `.text` for the IMU modules. RAM: `imu_terrain_t` 3.3 kB + snapshot 0.5 kB + stack 2 kB + queue 0.6 kB ≈ 6.4 kB.

---

## Part 4 — Code

All sources are in this branch. The complete listing (every `.c`/`.h` with its filename) is in [`BUDDY4_FULL_LISTING.md`](BUDDY4_FULL_LISTING.md).

| File | Purpose |
|---|---|
| `include/imu/imu_types.h` | Common types and enums (also included by the MC) |
| `include/imu/lsm303dlhc_regs.h` | Register map |
| `include/imu/imu_config.h`, `src/imu/imu_params.c` | Pins, defaults, `imu_params_t`, validation (axis-map handedness check) |
| `include/imu/imu_hal.h`, `src/imu/imu_hal_pico.c` | Pico SDK I²C on GP16/GP17, bus-clear, ms/µs time |
| `include/imu/imu_lsm303dlhc.h`, `src/imu/imu_lsm303dlhc.c` | Driver |
| `include/imu/imu_math.h`, `src/imu/imu_math.c` | Math |
| `include/imu/imu_calibration.h`, `src/imu/imu_calibration.c` | Calibration |
| `include/imu/imu_processing.h`, `src/imu/imu_processing.c` | Signal processing and detectors |
| `include/imu/imu_hump.h`, `src/imu/imu_hump.c` | Hump FSM and measurement |
| `include/imu/imu_events.h`, `src/imu/imu_events.c` | Event message and dispatcher |
| `include/imu/imu_terrain.h`, `src/imu/imu_terrain.c` | Subsystem state machine, status, telemetry |
| `include/imu/imu_rtos.h`, `src/imu/imu_rtos.c` | µT-Kernel task and public thread-safe API |
| `tools/buddy4_bringup/bringup_main.c` | Bring-up / calibration logger firmware |
| `tools/buddy4_bringup/analyse_log.py` | Log statistics and threshold suggestions |
| `tests/imu/*` (retired) | Historical standalone tests, simulator, fake kernel and replay tool |

---

## Part 5 — Build and installation

### Firmware (Pico SDK, as configured in the repo's `.vscode/settings.json`)

```bash
git checkout buddy4-imu
export PICO_SDK_PATH=~/.pico-sdk/sdk/<version>      # VS Code extension layout
mkdir -p build && cd build
cmake -G Ninja ..                                    # PICO_BOARD=pico_w is set in CMakeLists.txt
ninja buddy4_imu_bringup                             # → build/buddy4_imu_bringup.uf2
ninja INF2004_Embedded_Systems                       # team image (unchanged)
```

Flashing: hold **BOOTSEL**, plug in USB, then copy `buddy4_imu_bringup.uf2` to the `RPI-RP2` drive (or `picotool load -x buddy4_imu_bringup.uf2`). Open the USB serial port (e.g. `screen /dev/ttyACM0 115200`, PuTTY, or VS Code Serial Monitor) and press `h`.

### CMake changes (appended to the top-level `CMakeLists.txt`; nothing existing was modified)

- `buddy4_imu`: INTERFACE library containing the driver, algorithms and state machine. It links `pico_stdlib hardware_i2c hardware_gpio hardware_timer m`.
- `buddy4_imu_rtos`: defined only when `-DUTKERNEL_INCLUDE_DIRS=...` is given. It adds `imu_rtos.c` and `PROHIBIT_DEF_SIZE_T`.
- `buddy4_imu_bringup`: a separate executable with its own `main()`.
- `-DBUDDY4_RTOS_COMPILE_CHECK=ON`: compiles `imu_rtos.c` against the kernel headers without linking (CI check).

Example (verified with the TRON RP2040 port):

```bash
cmake -G Ninja .. -DUTKERNEL_INCLUDE_DIRS="$MTK/include;$MTK/config;$MTK" \
      -DUTKERNEL_DEFINITIONS=_PICO_RP2040_ -DBUDDY4_RTOS_COMPILE_CHECK=ON
ninja buddy4_imu_rtos_check
```

### Integrating into the team firmware (once the course µT-Kernel template is in the repo)

```cmake
target_link_libraries(INF2004_Embedded_Systems buddy4_imu_rtos)
```

```c
/* in usermain() / the initialisation task, after the MC's objects exist */
#include "imu/imu_rtos.h"
imu_rtos_config_t cfg;
imu_rtos_default_config(&cfg);
cfg.task_priority      = 9;               /* CONFIRM */
cfg.terrain_event_q    = 0;               /* 0 = Buddy 4 creates it; or pass the MC's ID */
cfg.mc_flag_id         = mc_event_flag;   /* CONFIRM */
cfg.collision_flag_ptn = MC_FLG_COLLISION;
cfg.fault_flag_ptn     = MC_FLG_IMU_FAULT;
(void)imu_rtos_start(&cfg);
```

**Missing dependencies (stated explicitly):**
- The course's µT-Kernel template (startup, `usermain`, kernel library) is **not in the repo**, so the full RTOS image was **not linked**.
- Under µT-Kernel, `pico_stdlib`'s `sleep_ms` must not be used in tasks. Buddy 4 does not use it; the bring-up tool is bare-metal only.

### Host tests (Linux/macOS/WSL, gcc or clang)

The following commands are historical and no longer apply. The current
51-case suite is run from the repository root with
`bash testing/subsystem4/host/run_host_tests.sh normal` (or `sanitize`).

```bash
cd tests/imu     # historical path; retired
make             # 113 tests
make asan        # AddressSanitizer + UndefinedBehaviorSanitizer
make coverage    # per-file line / branch coverage (gcov)
make tidy        # clang-tidy (bugprone, cert, clang-analyzer)
make replay-demo # offline replay tool on a synthetic log
./buddy4_tests -f bb1 -v    # filter by name, print every event
```

---

## Part 6 — Testing

### Strategy

| Level | Technique | File | Tests |
|---|---|---|---|
| Unit (white-box) | Statement/branch coverage, boundary values | `test_unit_driver.c`, `test_unit_math_cal.c`, `test_unit_processing.c`, `test_unit_hump.c`, `test_unit_events.c` | 62 |
| System (black-box) | Equivalence partitioning, boundary values, state-transition, use-case scenarios | `test_blackbox.c` (public API, events and status only) | 29 |
| Robustness / "penetration" | Fault injection, fuzzing, abuse cases | `test_robustness.c` | 16 |
| Integration (RTOS wrapper) | Fake µT-Kernel (`fake_tk/`), same struct layouts and constants | `test_rtos.c` | 6 |
| Dynamic analysis | ASan + UBSan over the whole suite | `make asan` | all |
| Static analysis | clang-tidy, `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wdouble-promotion` (host and ARM) | `make tidy` | — |

**Test fixtures:**
- `sim_lsm303.c` is a register-level LSM303DLHC: CTRL/STATUS/FIFO stream mode with overrun, 12-bit left-justified little-endian accelerometer, big-endian X/Z/Y magnetometer with −4096 overflow, ID registers, and fault injection.
- `scenario.c` is a rigid-car physics model: wheelbase, raised-cosine humps, pitch = asin(Δz/L), vertical and centripetal acceleration, impacts, mounting matrix, hard/soft iron, magnetic disturbance, and noise.

### The 16 required scenarios

| # | Scenario | Test(s) | Expected result (asserted) |
|---|---|---|---|
| 1 | Init / communication failure | `bb01`, `bb02`, `t_init_failures` | Stays INITIALISATION; IMU_FAULT once; recovers when the device appears; wrong ID → `IMU_ERR_ID` |
| 2 | Stationary | `bb04`, `bb05` | READY after ≈2 s (100 samples); \|pitch\|,\|roll\| < 0.5° for 10 s; STATIONARY; no events |
| 3 | Straight line | `bb06` | STEADY_MOTION (HIGH with hint); no hump, turn, tilt or impact events |
| 4 | Accel / decel | `bb07`, `bb07b` | ACCELERATING → STEADY → DECELERATING → STATIONARY; no hump counted (with or without hint) |
| 5 | Left / right turns | `bb08`, `t_turn_left_right`, `t_turn_rate_wraparound` | One LEFT END (Δ ≈ −90 ± 15°), one RIGHT END (Δ ≈ +90 ± 15°); correct across 359°→0° |
| 6 | Small hump (12 mm) | `bb09` | 1 DETECTED, 1 PEAK, 1 COMPLETED; θ within 25 % of truth |
| 7 | Large hump (30 mm) | `bb10` | θ within 20 %; height 30 ± 7 mm |
| 8 | Multiple humps, retain highest | `bb11`, `t_multiple_retain_max` | 3 valid; max = #2; `b_new_max` exactly twice; telemetry `hump_peak` = max |
| 9 | Noise / false humps | `bb12`, `t_spike_rejected`, `t_slope_and_no_descent` | 0.06 g vibration + 2 mm ridges + 0.8 g knocks → 0 humps, 0 impacts |
| 10 | Short sudden impact | `bb13`, `t_impact_detector` | CRITICAL IMPACT, flag notified, FAULT_SAFE/COLLISION, SUDDEN_IMPACT; no silent resume |
| 11 | Repeated impacts | `bb14`, `t_impact_cooldown` | 5 scrapes in 0.5 s → 1 event (+1 after cool-down with suppressed ≥ 3); escalation still reported |
| 12 | Tilt | `bb15`, `t_tilt_detector` | 20° roll → START/END; 40° → CRITICAL + flag; debounce and hysteresis respected |
| 13 | Disconnection / invalid | `bb16`, `bb18`, `p01`, `p03` | Bus stuck → SENSOR_FAULT → FAULT_SAFE within 200 ms; frozen output and 0 g output detected |
| 14 | Recovery / persistent | `bb16`, `bb17`, `bb19` | Recovered with calibration kept; persistent → one PERSISTENT event and never MONITORING in 60 s; 1-in-7 NACKs absorbed |
| 15 | Events / queue overflow | `bb20`, `t_overflow_policy`, `t_pending_ring_bound`, `r05` | Detection unaffected; non-critical dropped and counted; critical deferred and delivered in order; bounded |
| 16 | Mission restart | `bb21` | MISSION_START clears last/max (telemetry −1); next hump is index 1 |

### Robustness / "penetration" tests (adversarial input surfaces)

Buddy 4 has no network surface; its inputs are the I²C bus, the inter-task API, the encoder hint, the parameter block, the time base and the event queue.

| Test | Attack / fault | Pass criterion |
|---|---|---|
| `p01` | 30 s of random bytes on every I²C read | No crash/UB, no NaN in state, no fake hump, bounded event count |
| `p02` | Failure ratio sweep 100 % … 8 % (NACK/timeout) | 100 % → reported fault; ≤ 50 % isolated → absorbed by retries |
| `p03` | Saturated ±100 g / 50 G inputs | Sane state, heading INVALID |
| `p04` | NULL pointers, invalid command masks, out-of-range enums on every public function | `IMU_ERR_PARAM`, never a crash |
| `p05` | Random command flood every cycle for 15 s; RESET storm | Bounded events; never a fake hump; recovers afterwards |
| `p06`, `p07` | Spoofed hints (random speed/accel/timestamps), replayed or future hints | Rejected or ignored; the real hump is still detected |
| `p08` | 300 fuzzed parameter sets (NaN, negative, mirrored axis maps) | Either `IMU_ERR_PARAM` or runs safely |
| `p09` | 32-bit millisecond wrap during a hump | Hump detected; no fault |
| `p10` | Time jumps ±30–50 s | Filters re-seeded; no false events |
| `p11` | Consumer never reads + impact storm | Pending ring ≤ 4; direct flag still works; stays in safe state while being hit |
| `p12` | NaN/Inf/1e30 into every processing entry point | Ignored; no NaN propagation |
| `p13`, `p14` | Magnetometer overflow stream; frozen-output boundary (N−1 / N) | No false fault; exact boundary |
| `p15`, `p16` | Memory budget; 10-minute soak with 8 humps and turns | Sizes within budget; counts equal ground truth |

### Results (this branch)

```
113 tests, 2939 assertions, 0 FAILED        (gcc 13, -O2 -Werror)
113 tests, 2939 assertions, 0 FAILED        (ASan + UBSan)
clang-tidy: no findings
Line coverage: params 97.8 %, math 98.1 %, driver 98.6 %, calibration 96.4 %,
processing 95.8 %, hump 93.1 %, events 100 %, terrain 98.0 %, rtos 88.3 %
```

Defects found and fixed by these tests during development:
1. A frozen sensor passed recovery verification (now requires changing data).
2. An acceleration artefact blocked the next real hump.
3. Fast humps were under-read because the peak was tracked only after confirmation.
4. PERSISTENT was never reported when re-initialisation kept succeeding but verification failed.
5. The STOPPED state was not published after shutdown.

---

## Part 7 — Hardware calibration and threshold tuning

Use `buddy4_imu_bringup.uf2`. Wheels off the ground or motors disabled unless a step says otherwise. Save the terminal output to a file for `analyse_log.py`.

**1. Wiring and bus.**
- Connect the LSM303DLHC breakout to Grove 4 (VCC 3V3, GND, SDA→GP16, SCL→GP17). Check the breakout has I²C pull-ups (4.7–10 kΩ).
- Power up and press `s`.
- Expect `state=MONITORING` within ~3 s, `i2c failed=0`.
- If you see `INIT_FAILED err=-4` (ID), it is not an LSM303DLHC. If you see `err=-2/-3`, check wiring, pull-ups, and 3V3.

**2. Axis verification** (sets `axis_map`/`axis_sign` in `imu_params_default`). Press `w` (RAW) and do the following:

| Action | Expected after correct mapping | If wrong |
|---|---|---|
| Car level | RAW shows ≈ +1 g on the sensor axis pointing **up** | That sensor axis becomes `axis_map[2]` (sign = sign of the reading) |
| Lift the **rear** (nose down) | Sensor axis pointing **forward** goes **negative** | That axis becomes `axis_map[0]` |
| Lift the **right** side | Sensor axis pointing **left** goes **positive** | That axis becomes `axis_map[1]` |
| Then press `l` (CSV): lift the front | `pitch` positive | Fix the signs |
| Lift the left side | `roll` positive | |
| Rotate the car left (CCW seen from above) | `heading` decreases | |

`imu_params_validate()` rejects a mirrored (left-handed) map, which would swap left and right turns.

**3. Stationary accelerometer calibration** (automatic at every power-up):
- Motors off, car on the actual track surface, nobody touching it, for 3 s.
- Record `g_ref`, `noise`, and `mount_tilt` (`s`) over 5 power cycles. Expect g_ref 0.95–1.05, noise < 5 mg, and mount_tilt repeatable within ±0.3°.
- A large mount_tilt (> 10°) means the bracket is crooked. That is compensated, but check the axis map.
- Optional six-position bench test (each axis ±1 g): bias = (r⁺ + r⁻)/2 → `accel_bias_g`.

**4. Magnetometer calibration with motor interference.**
- Mount the IMU as far from the motors and battery as practical (≥ 8–10 cm) and twist the motor leads.
- In the integrated firmware, the MC commands Buddy 2 to spin in place slowly (≤ 45 °/s, ≥ 1.25 turns) **with the motors running at normal PWM**. This captures the motor magnets (hard iron) and typical current fields.
- In bring-up: press `b`, rotate the car by hand slowly through ≥ 1.25 turns (motors running if safe), then press `e`, then `s`.
- Copy `hard_iron`, `soft`, and `ref` into the `mag_*` fields of `imu_params_default`, and set `b_mag_cal_valid = true`.
- Validate at 8 compass directions against a phone compass: errors should be < 10°. Note that only heading **changes** matter for turn detection.
- Then run the motors at 0 / 50 / 100 % PWM while stationary and watch `|B|` (CSV column 13). If it moves more than `mag_field_tol_frac` (25 %), turns are flagged DISTURBED. Either relocate the IMU, or accept that turn events will be unavailable at high PWM.

**5. Threshold tuning.**
Record at least 10 runs each of: flat straight (2–3 speeds), 90° turns left and right, humps of known height, and controlled impacts. Then run:

```bash
python3 tools/buddy4_bringup/analyse_log.py capture.txt --still 0:3000 --true-height 20
```

| Parameter | Rule of thumb (validate!) |
|---|---|
| `hump_ascent_enter_deg` | > 99th percentile of \|pitch\| on flat runs (script prints it) + 1°, and < 60 % of the smallest hump's peak |
| `hump_min_peak_deg` | Between the ascent threshold and the smallest real hump peak |
| `hump_descent_enter_deg` | ≈ 40–60 % of the smallest real hump's trough |
| `hump_crest_max_ms` | ≥ 1.3 × L / v_min (time from crest to rear wheels climbing) |
| `stationary_vib_g` | 3× the stationary noise (motors off) and below the vibration while driving |
| `accel_enter_g` | Above the 99th percentile of \|fwd_g\| while cruising |
| `turn_enter_dps` | ≈ 50 % of Buddy 2's commanded turn rate; `turn_exit_dps` ≈ 50 % of enter |
| `impact_minor_g` | > 2 × the largest \|hp\| peak seen on hump edges and floor joints |
| `impact_critical_g` | From controlled bumps (below) |

Historical replay command (retired): `tests/imu/buddy4_replay capture.txt hump_ascent_enter_deg=2.5 ...` re-ran the standalone algorithms on recorded data. This is not part of the current 51-case suite.

**6. Hump repeatability and height accuracy (A-B4-2, the terrain analysis report).**
- Build ≥ 3 humps of known height (e.g. 10, 20, 30 mm; measure with a caliper) and record the profile length.
- Do 10 passes per hump at the mission speed, and 5 passes at ±30 % speed.
- Record for each pass: detected (y/n), θ_peak, θ_min, duration, `h_mm`.
- Report:
  - mean, SD and CV of θ_peak per hump
  - separability: (mean difference) / (pooled SD) > 3 means the highest hump is reliably identified
  - missed detections and false positives (≥ 20 flat runs → 0 expected)
  - least-squares fit of the true height h against L·sin θ, giving `gain` and `offset`
  - validation error on a hump not used in the fit
- Then set `b_hump_height_calibrated = true` (confidence rises to MEDIUM).

**7. Controlled, safe impact tests.**
- Never at full speed and never by driving into rigid objects.
- Lift the wheels or disable the motors; tap the bumper with a padded mallet, or roll the car by hand into a foam block at 0.1–0.2 m/s.
- Record `peak_mg` and the horizontal share. Set `impact_critical_g` between the worst hump/kerb pulse and the softest "should stop" bump.

**8. Disconnection and recovery.**
- With the wheels off the ground, unplug the Grove cable while logging. Expect an `IMU_FAULT ... status=2` (FAULT_RAISED) event within ~0.2 s.
- Replug. Expect `status=4` (RECOVERED) within ~0.5–2 s, with calibration kept.
- Hold the cable unplugged for over 30 s. Expect one `status=3` (PERSISTENT).

### Terrain analysis report — template

| Hump | True h (mm) | Length (mm) | Passes | Detected | θ_peak mean ± SD (°) | CV % | Est. h mean ± SD (mm) | Error (mm) |
|---|---|---|---|---|---|---|---|---|
| A | | | 10 | | | | | |
| B | | | 10 | | | | | |
| C | | | 10 | | | | | |
| Flat runs | — | — | 20 | false positives: | | | | |

Include the speed sensitivity (θ_peak versus speed), the final thresholds and the reasoning behind each, the noise figures from step 3, and the limitations from Part 2.

---

## Part 8 — Team integration

### Mission Controller (shared)

- **Receives** `imu_event_t` from `terrain_event_q`:
  - get the ID with `imu_rtos_terrain_event_q()`, or pass the MC's own ID in `cfg.terrain_event_q`
  - receive with `tk_rcv_mbf(id, &evt, timeout)`
  - check `seq` gaps for drops
- **Direct notification:** `tk_set_flg(cfg.mc_flag_id, cfg.collision_flag_ptn)` on a CRITICAL impact, and `fault_flag_ptn` on IMU_FAULT or a critical tilt. Wait with `tk_wai_flg(..., TWF_ORW | TWF_BITCLR, ...)`.
- **Expected reactions** (DR §5):
  - `IMPACT` CRITICAL → request STOP from Buddy 2 → system FAULT. After the stop is confirmed and the cause cleared, call `imu_clear_fault()`.
  - `IMU_FAULT` FAULT_RAISED → safe stop (critical sensor lost). RECOVERED means the sensor is OK again; resuming the mission is the MC's decision.
  - `TILT` CRITICAL → stop. MINOR → log only.
  - `HUMP_*`, `TURNING`, `MOTION`, `STATUS` → informational and telemetry. `STATUS READY` means the IMU is calibrated (part of INIT_SUCCESS).
- **Commands:**
  - `imu_mission_start()` on START_RUN (resets last/max)
  - `imu_request_reset()` on system RESET
  - `imu_recalibrate()` only while stopped (READY)
  - `imu_mag_cal_begin()` / `imu_mag_cal_end()` around the spin manoeuvre
- **Queries:** `imu_get_motion`, `imu_get_tilt`, `imu_get_heading`, `imu_get_hump_state`, `imu_get_last_hump`, `imu_get_max_hump`, `imu_get_health`, `imu_get_status`.

### Buddy 1 (telemetry)

- Call `imu_get_telemetry(&t)` from the telemetry task at 10 Hz. It is non-blocking (mutex ≤ 10 ms) and never touches the sensor.
- `imu_terrain_telemetry_json(&t, buf, len)` writes the fragment `"hump_peak":<cdeg>,"hump_peak_mm":<mm|-1>,"hump_count":n,"imu_state":s,"motion":m,"pitch":<cdeg>,"heading":<cdeg|-1>,"imu_ok":0/1` (≈ 110 B).
  - Use only `"hump_peak"` if the 256 B budget (NFR10) is tight.
- **CONFIRM:** whether `hump_peak` should be centi-degrees (measured) or mm (estimate).
- Optional push model: `cfg.p_telemetry_hook`, called from the IMU task every `telemetry_period_ms`. It must use `TMO_POL`.
- Buddy 4 has **no** MQTT/Wi-Fi code.

### Buddy 2 (motion) — optional, improves accuracy

- After each control update (or every ≤ 50 ms), Buddy 2 or the MC calls `imu_set_motion_hint(&h)` with `speed_mm_s`, `accel_mm_s2` (if available) and `timestamp_ms`.
- Effects:
  - removes the acceleration-induced pitch error
  - gives STATIONARY/STEADY with HIGH confidence
  - blocks hump detection while the car is not moving
  - enables the integrated height method and the hump-length check
- Hints older than 200 ms, or implausible ones (> 3 m/s, > 20 m/s²), are ignored.
- Buddy 4 never calls any motor function.

### Kernel configuration required (TRON µT-Kernel 3.0 `config.h`)

- `CNF_MAX_MBFID ≥ 8` (DR §4 queues), `CNF_MAX_MTXID ≥ 4 + others`, `CNF_MAX_CYCID ≥ 2`, `CNF_MAX_FLGID` for the MC flag
- `CNF_TIMER_PERIOD` divides 20 ms
- `DEVCNF_USE_IIC 0` in `config_device.h`: the BSP I²C driver for unit 0 would mux GP8/GP9 (Buddy 2's motor pins)
- `PROHIBIT_DEF_SIZE_T` for every file that mixes kernel and SDK headers

### Items requiring confirmation with the integration team

1. Final numeric priorities.
2. Who creates `terrain_event_q`.
3. Event-flag ID and bits.
4. Unit of `hump_peak`.
5. The course template's µT-Kernel / Pico SDK coexistence (startup, SysTick versus SDK timer).
6. Measured wheelbase and IMU position (`IMU_CFG_WHEELBASE_MM`, `IMU_CFG_IMU_POS_FRAC`).
7. IMU mounting orientation (axis map).

---

## Part 9 — Compliance review

| Requirement | Status | Evidence / remaining work |
|---|---|---|
| FR7 (acquire, tilt, motion state) | **Implemented, simulation-verified** | `bb04–bb08`, `bb15`. Hardware: axis and noise verification (Part 7, steps 2–3) |
| FR8 (detect humps, peak value, retain max) | **Implemented, simulation-verified** | `bb09–bb12`, `bb21`, `bb24`. Absolute height is an *estimate* with LOW confidence until the gain is fitted on real humps |
| FR9 (turning, impact/collision) | **Implemented, simulation-verified** | `bb08`, `bb13`, `bb14`. Turn rate is heading-derived (not gyro); thresholds need real impacts |
| NFR5 (reliable, stable, consistent) | **Design and tests in place; hardware evidence pending** | Repeatability spread 17 % over 0.12–0.45 m/s in simulation; real CV needs the Part 7 step 6 data |
| NFR11 (C, Pico SDK, µT-Kernel) | **Met for code; full RTOS link pending** | Firmware builds; `imu_rtos.c` compiles against real µT-Kernel headers; course template not in repo |
| NFR12 (Barr C, repo structure) | **Largely met** | Fixed-width types, `p_`/`b_`/`g_` naming, braces everywhere, `(void)` casts, no heap, include guards, `static` internals, explicit `const`. **Deviations:** `union` in `imu_event_t` (clarity of message payloads); early `return` for argument checks; a few files exceed 80 columns in tests only; `float` used (RP2040 ROM float) |
| NFR13 (clear interfaces) | **Met** | Black-box tests use public headers only; the MC needs only `imu_events.h`/`imu_rtos.h` |
| NFR14 (resources) | **Met (estimated)** | ≈ 7 % CPU estimated, ≈ 6.4 kB RAM, no heap, bounded queues. Measure `max_cycle_us` on target |
| NFR15 (coordinated vehicle) | **Interfaces ready; integration pending** | Queue, flag, commands, telemetry, hint |
| DR §6.4 statechart | **Met** | Mapping table in Part 3; transitions verified by trace assertions |
| DR §8.2 decisions (no motor control, configurable thresholds) | **Met** | No motor references; every threshold is in `imu_params_t` |
| PW Buddy 4 tasks (integration, calibration, tilt, hump, peak, motion, collision, turn rate, events) | **Implemented** | See Part 1 |
| SB deliverables: processing module, hump algorithm, **terrain analysis report** | Module and algorithm done; **report needs track data** | Template and procedure in Part 7 |

**Cannot be verified without hardware:**
- real I²C timing and pull-ups on Grove 4
- whether the DLHC clone on your breakout matches the datasheet (ID registers, FIFO behaviour)
- motor magnetic interference
- real hump, turn and impact signatures, and every threshold
- CPU time on the RP2040
- coexistence with the course µT-Kernel template

**Physically impossible with this sensor alone:**
- true angular rate (no gyroscope)
- distinguishing standing still from constant velocity
- an exact absolute hump height without the wheelbase and hump-geometry assumptions or encoder speed

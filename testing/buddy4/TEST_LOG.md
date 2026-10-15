# Buddy 4 — current implementation test log

New test cases reside here per the user's folder choice. IDs S4-U01/02 and
S4-U003–S4-U115 remain reserved by the historical records in
`testing/subsystem4/`; none are reused.

## Automated cases

Run `bash testing/buddy4/run_host_tests.sh normal` (or `sanitize`).
S4-U149–S4-U161 are host-only API simulations in `runtime_main.c`.
The other 38 cases are in `buddy4_cases.c` and also compile into TEST=1.
All numerical tolerances and inputs are synthetic regression fixtures.

| ID | Case | Expected result |
|---|---|---|
| S4-U116 | Invalid/null configuration | Reject invalid initialization. |
| S4-U117 | Stationary calibration | Two-sample fixture completes with CALIBRATED event. |
| S4-U118 | Calibration movement | Reset the accumulation window. |
| S4-U119 | Pitch convention | Synthetic 30 degree nose-up reports 30 degrees within 0.01. |
| S4-U120 | Roll convention | Synthetic 30 degree roll reports 30 degrees within 0.01. |
| S4-U121 | Hump state sequence and integral | Ascent/peak/descent/monitor; 10 mm fixture peak. |
| S4-U122 | Smaller second hump | Retain 10 mm session maximum. |
| S4-U123 | Larger next hump | Increase session maximum to 20 mm. |
| S4-U124 | No distance input | No valid measured height or maximum. |
| S4-U125 | Missing distance during ascent | Invalidity persists through the hump. |
| S4-U126 | Hump duration limit | Latch fault on unfinished hump. |
| S4-U127 | Impact candidate | Latch FAULT and IMPACT. |
| S4-U128 | Fault recovery | Good data does not clear the latch. |
| S4-U129 | Duplicate timestamp | Latch fault. |
| S4-U130 | Timestamp wrap | Accept forward elapsed time across uint32 wrap. |
| S4-U131 | Stale timestamp | Reject gap above configured maximum. |
| S4-U132 | Nonfinite acceleration | Reject NaN and latch fault. |
| S4-U133 | Reverse distance | Reject negative forward-distance input. |
| S4-U134 | Stationary ambiguity | Without distance, gravity alone yields UNKNOWN motion. |
| S4-U135 | Uncalibrated magnetometer | Do not mark heading valid. |
| S4-U136 | Heading wrap | 179 to -179 degrees produces shortest +100 deg/s fixture rate. |
| S4-U137 | Missing magnetic update | Clear heading validity and do not reuse stale yaw. |
| S4-U138 | Sensor configuration | Match datasheet profile and verify writes by readback. |
| S4-U139 | Acceleration conversion | Decode signed little-endian HR values in g. |
| S4-U140 | Magnetic conversion | Decode X,Z,Y big-endian values with axis-specific gains. |
| S4-U141 | Magnetic data ready | Missing DRDY never reuses old magnetic data. |
| S4-U142 | Acceleration data ready | Return NOT_READY when acceleration is stale. |
| S4-U143 | Identity mismatch | Reject wrong magnetic ID. |
| S4-U144 | Register readback mismatch | Reject initialization. |
| S4-U145 | Bus failure | Return failure without committing partial sample. |
| S4-U146 | Magnetic overflow | Keep acceleration usable; invalidate heading. |
| S4-U147 | Acceleration saturation | Reject saturated samples. |
| S4-U148 | Driver null arguments | Return failure safely. |
| S4-U149 | Mutex creation failure | Propagate error; do not start task. |
| S4-U150 | Fault flag creation failure | Propagate error; do not start task. |
| S4-U151 | Event buffer creation failure | Propagate error; do not start task. |
| S4-U152 | Task creation failure | Propagate error; do not start task. |
| S4-U153 | Two-phase startup | All four objects exist before explicit task start. |
| S4-U154 | Task start failure | Return the actual tk_sta_tsk error. |
| S4-U155 | Sensor open failure | Latch fault, publish invalid status, exit. |
| S4-U156 | Full ordinary queue | Count drops; independent fault latch remains readable twice. |
| S4-U157 | No fresh sensor samples | Timeout, latch fault, exit. |
| S4-U158 | Task delay API error | Latch fault and invalidate snapshot. |
| S4-U159 | Work overrun | 25 ms fixture exceeds 20 ms work budget and faults. |
| S4-U160 | Mutex API timeout | Status/speed APIs propagate timeout. |
| S4-U161 | Public input validation | Reject nulls/reverse speed; permit explicit speed revocation. |
| S4-U162 | Week 6 peak-time reporting | Highest estimate is visible at PEAK, before descent. |
| S4-U163 | Dynamic acceleration at entry | Do not restore height validity when ascent starts. |
| S4-U164 | Gap boundary | Allow exactly maximum_gap_ms. |
| S4-U165 | Null processing inputs | Fail safely. |
| S4-U166 | Complete HR numeric domain | Verify all 4096 encodings, sign and saturation limits. |

## Pending hardware cases

Do not run without authorization. Agree numerical accuracy/timing limits
before recording an acceptance PASS; none are invented here.

| ID | Procedure | Expected evidence / result |
|---|---|---|
| S4-U170 | Inspect exact breakout, supply and bus pull-ups; verify GP16/17 and axis mapping with power safely isolated | Correct part, exclusive pins, common ground, compatible 3.3 V logic; save wiring record |
| S4-U171 | Stationary calibration, moved startup, known positive/negative tilt fixtures | Calibration only under intended stationary conditions; measured bias/noise and tilt error within team-approved limits |
| S4-U172 | Repeated known humps with differing heights, lengths and slopes at measured speeds | Correct highest physical hump, peak-time event, recorded height errors within agreed limits; invalid estimates clearly flagged |
| S4-U173 | Controlled turns with motors energized; magnetic calibration and interference checks | Recorded heading/turn error and invalidity behavior within agreed limits; no gyro claim |
| S4-U174 | Restrain vehicle, inject bus disconnect/stuck bus, task load and full normal event buffer | Measured sampling/jitter and independent controller/Buddy 2 fault-to-stop latency meet reviewed limits |
| S4-U175 | Authorized restrained impact and no-impact rough-terrain trials | Recorded missed/false impact rate; no unsafe automatic restart; stop path preserves Buddy 2 ownership |

Physical evidence belongs in logs and the shared videos/README.md index.

## Results (append only)

Subject base HEAD: `a405c5aa2752e39bdcd3d4e0b415d2126158ea15`,
branch `buddy4-imu`, with uncommitted S4/shared-build/test changes.
No commit/stage/push approval was given. Each saved console lists dirty state,
compiler, exact commands and SHA-256 hashes identifying the tested sources.
This explicitly records a working tree rather than claiming a committed test.

| Date | Scope | Result | Evidence / qualification |
|---|---|---|---|
| 2026-10-15 | Initial Make attempt | FAIL (compile) | `make -j8`: missing stddef before PROHIBIT_DEF_SIZE_T bridge; fixed before subsequent builds. Output in session transcript, no saved acceptance log. |
| 2026-10-15 | First host launch | BLOCKED by sandbox | Process-substitution log capture denied; rerun with approved execution. No test ran. |
| 2026-10-15 | S4-U116–S4-U148 | PASS, 33 host cases | [normal.K4iFbV](logs/normal.K4iFbV/console.txt); early implementation before added RTOS/peak-boundary cases |
| 2026-10-15 | S4-U116–S4-U161 | PASS, 46 host cases | [normal.Z2SqVv](logs/normal.Z2SqVv/console.txt); deterministic fake kernel, not SMP evidence |
| 2026-10-15 | Four integrated Make profiles | PASS, compilation/link/UF2 generation | [build.e1hvBA](logs/build.e1hvBA/console.txt); default, TEST=1, S4_IMU=1, TEST=1 S4_IMU=1; source hashes identify this intermediate revision |
| 2026-10-15 | S4-U116–S4-U166 | PASS, 51 host cases, ASan + UBSan | [sanitize.cscKF0](logs/sanitize.cscKF0/console.txt); no sanitizer diagnostics; before final descent integration guard |
| 2026-10-15 | S4-U116–S4-U166 | PASS, 51 host cases, final C sources | [normal.50scyT](logs/normal.50scyT/console.txt); strict C99 compilation, zero failures |
| 2026-10-15 | S4-U116–S4-U166 | PASS, 51 host cases, final C sources, ASan + UBSan | [sanitize.brjx0W](logs/sanitize.brjx0W/console.txt); no sanitizer diagnostics |
| 2026-10-15 | Four integrated Make profiles, final C sources | PASS, compilation/link/UF2 generation | [build.FCBUK8](logs/build.FCBUK8/console.txt); default, TEST=1, S4_IMU=1, TEST=1 S4_IMU=1; Arm GNU 15.3.1, SDK at recorded local path |
| 2026-10-15 | Script/whitespace/static checks | PASS within stated scope | `bash -n testing/buddy4/run_host_tests.sh testing/buddy4/run_builds.sh`; `git -c core.whitespace=-blank-at-eof diff --check`; no new IMU lines above 80 columns or functions above 100 lines. EOF exception explained in BARR_REVIEW.md. |

No on-target RTOS run, flash, electrical verification, sensor calibration,
height accuracy measurement or physical test has been performed.

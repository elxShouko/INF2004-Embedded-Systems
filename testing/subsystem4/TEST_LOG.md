# Subsystem 4 — test log

**Subsystem:** Buddy 4 · **Code:** `app_program/subsystem4/`

See `testing/README.md` for the shared convention. This is the single registry
for the 82 current-firmware host cases, retained monitor checks and pending
hardware checks. Historical IDs S4-U003–S4-U115 are retired/reserved and are
not reused. S4-U167–S4-U169 remain unassigned.

## Layout and commands

```text
testing/subsystem4/
├── TEST_LOG.md          all test cases, instructions and results
├── host/                all current cases, test doubles and runners
└── logs/                saved evidence
```

From the repository root:

```sh
bash testing/subsystem4/host/run_host_tests.sh normal
bash testing/subsystem4/host/run_host_tests.sh sanitize
bash testing/subsystem4/host/run_builds.sh
```

Both host commands run all **82 current-firmware cases**: 57 core/driver
checks, 23 runtime/bridge checks and 2 transport checks. No legacy sources
or cases
are built or extracted.

The build runner compiles GNU Make profiles: default, `TEST=1`, `S4_IMU=1`,
and `TEST=1 S4_IMU=1`. Defaults remain SMP=1, CONSOLE=usb_cdc, WIFI=cyw43,
NET=none, MQTT=0, CRASH_REPORT=0. No flashing or hardware access is performed.
The default S4_IMU=0 keeps sensor pin access disabled.

The extra `testing/buddy4/` and `tests/imu/` folders are retired. The user
chose to keep the 51 then-current cases; this fix adds 24 new current cases.
The 113 legacy cases,
separately deleted guides/reviews and older evidence files are not restored.
Their tracked versions remain recoverable from Git HEAD `4240895`.
Historical result rows below are retained as records, not current coverage.

## Test cases

| ID | What is tested | Requirement / interface | Method | Steps / how | Expected result |
|---|---|---|---|---|---|
| S4-U01 | Message buffer maximum line | Retained IF-03 monitor | auto (target) | `app_program/subsystem4/subsystem4_test.c` with TEST=1 | Buffer maximum equals S4_SUMMARY_MAX |
| S4-U02 | LED pauses and resumes every 6 reports | Retained IF-05 monitor | manual | Authorized normal-image run; watch LED and console for about 20 s | Console reports pause/resume; LED stops/starts |
| S4-U116 | Invalid/null configuration | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reject invalid initialization. |
| S4-U117 | Stationary calibration | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Two-sample fixture completes with CALIBRATED event. |
| S4-U118 | Calibration movement | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reset the accumulation window. |
| S4-U119 | Pitch convention | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Synthetic 30 degree nose-up reports 30 degrees within 0.01. |
| S4-U120 | Roll convention | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Synthetic 30 degree roll reports 30 degrees within 0.01. |
| S4-U121 | Hump state sequence and integral | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Ascent/peak/descent/monitor; 10 mm fixture peak. |
| S4-U122 | Smaller second hump | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Retain 10 mm session maximum. |
| S4-U123 | Larger next hump | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Increase session maximum to 20 mm. |
| S4-U124 | No distance input | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | No valid measured height or maximum. |
| S4-U125 | Missing distance during ascent | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Invalidity persists through the hump. |
| S4-U126 | Hump duration limit | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Latch fault on unfinished hump. |
| S4-U127 | Impact candidate | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Latch FAULT and IMPACT. |
| S4-U128 | Fault recovery | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Good data does not clear the latch. |
| S4-U129 | Duplicate timestamp | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Latch fault. |
| S4-U130 | Timestamp wrap | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Accept forward elapsed time across uint32 wrap. |
| S4-U131 | Stale timestamp | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reject gap above configured maximum. |
| S4-U132 | Nonfinite acceleration | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reject NaN and latch fault. |
| S4-U133 | Reverse distance | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reject negative forward-distance input. |
| S4-U134 | Stationary ambiguity | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Without distance, gravity alone yields UNKNOWN motion. |
| S4-U135 | Uncalibrated magnetometer | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Do not mark heading valid. |
| S4-U136 | Heading wrap | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | 179 to -179 degrees produces shortest +100 deg/s fixture rate. |
| S4-U137 | Missing magnetic update | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Clear heading validity and do not reuse stale yaw. |
| S4-U138 | Sensor configuration | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Match datasheet profile and verify writes by readback. |
| S4-U139 | Acceleration conversion | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Decode signed little-endian HR values in g. |
| S4-U140 | Magnetic conversion | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Decode X,Z,Y big-endian values with axis-specific gains. |
| S4-U141 | Magnetic data ready | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Missing DRDY never reuses old magnetic data. |
| S4-U142 | Acceleration data ready | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Return NOT_READY when acceleration is stale. |
| S4-U143 | Identity mismatch | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reject wrong magnetic ID. |
| S4-U144 | Register readback mismatch | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reject initialization. |
| S4-U145 | Bus failure | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Return failure without committing partial sample. |
| S4-U146 | Magnetic overflow | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Keep acceleration usable; invalidate heading. |
| S4-U147 | Acceleration saturation | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Reject saturated samples. |
| S4-U148 | Driver null arguments | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Return failure safely. |
| S4-U149 | Mutex creation failure | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Propagate error; do not start task. |
| S4-U150 | Fault flag creation failure | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Propagate error; do not start task. |
| S4-U151 | Event buffer creation failure | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Propagate error; do not start task. |
| S4-U152 | Task creation failure | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Propagate error; do not start task. |
| S4-U153 | Two-phase startup | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | All four objects exist before explicit task start. |
| S4-U154 | Task start failure | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Return the actual tk_sta_tsk error. |
| S4-U155 | Sensor open failure | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Latch fault, publish invalid status, exit. |
| S4-U156 | Full ordinary queue | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Count drops; independent fault latch remains readable twice. |
| S4-U157 | No fresh sensor samples | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Timeout, latch fault, exit. |
| S4-U158 | Task delay API error | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Latch fault and invalidate snapshot. |
| S4-U159 | Work overrun | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | 25 ms fixture exceeds 20 ms work budget and faults. |
| S4-U160 | Mutex API timeout | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Status/speed APIs propagate timeout. |
| S4-U161 | Public input validation | Current: Runtime API / startup / faults | auto (host) | [runtime_main.c](host/runtime_main.c) via combined runner | Reject nulls/reverse speed; permit explicit speed revocation. |
| S4-U162 | Week 6 peak-time reporting | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Highest estimate is visible at PEAK, before descent. |
| S4-U163 | Dynamic acceleration at entry | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Do not restore height validity when ascent starts. |
| S4-U164 | Gap boundary | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Allow exactly maximum_gap_ms. |
| S4-U165 | Null processing inputs | Current: Core sensing / terrain contract | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Fail safely. |
| S4-U166 | Complete HR numeric domain | Current: LSM303DLHC register driver | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c) via combined runner | Verify all 4096 encodings, sign and saturation limits. |
| S4-U176 | Flat braking regression | Current height guard | auto (host, target) | Ten 20 ms samples at -0.2 g, speed decrements 39.2266 mm/s; [source](host/buddy4_cases.c) | PEAK state but no valid height or maximum; dynamic reason. Old core falsely reported 11.187815 mm. Original reported 2.313 mm trace unavailable. |
| S4-U177 | Flat acceleration | Current height guard | auto (host, target) | Accelerometer and wheel-speed change | MONITORING, dynamic reason, no maximum. |
| S4-U178 | Acceleration then braking | Current height guard | auto (host, target) | Successive signed speed changes | No valid maximum. |
| S4-U179 | Short pitch spike | Current height guard | auto (host, target) | One ascent sample with 40 ms synthetic dwell | PEAK state, persistence rejection, no maximum. |
| S4-U180 | Stale motion during candidate | Current height guard | auto (host, target) | Source age 41 ms | PEAK state, motion rejection, no maximum. |
| S4-U181 | Unavailable motion during candidate | Current height guard | auto (host, target) | Revoke motion-valid flag | PEAK state, no maximum. |
| S4-U182 | Reverse travel | Current height guard | auto (host, target) | Signed negative speed | PEAK state, reverse rejection, no maximum. |
| S4-U183 | In-place turn | Current height guard | auto (host, target) | Divergent wheels/unsupported plan | PEAK state, turn rejection, no maximum. |
| S4-U184 | Provisional retraction | Current height guard | auto (host, target) | Epoch changes after valid peak | Previously published maximum retracted with invalid event. |
| S4-U185 | Bad candidate after valid hump | Current height guard | auto (host, target) | Missing motion on second hump | Retain last completed 10 mm synthetic maximum. |
| S4-U186 | Valid hump after rejected candidate | Current height guard | auto (host, target) | Restore motion and cross larger synthetic hump | Maximum becomes 20 mm. |
| S4-U187 | Shallow/asymmetric descent | Current hump state | auto (host, target) | -3° descent then two level samples | DESCENDING then MONITORING; valid maximum closes. |
| S4-U188 | Untrusted traction | Current height guard | auto (host, target) | No controller trust assertion | No valid maximum; traction reason. |
| S4-U189 | Speed-band rejection | Current height guard | auto (host, target) | Drift above synthetic measured band | Dynamic reason; no maximum. |
| S4-U190 | Fresh forward Buddy 2 status | Current S2/S4 bridge | auto (host) | [runtime_main.c](host/runtime_main.c) | Signed 500 mm/s yields valid 10 mm increment and straight flag. |
| S4-U191 | Reverse Buddy 2 status | Current S2/S4 bridge | auto (host) | Negative wheel speeds | Signed speed retained, no forward distance. |
| S4-U192 | In-place Buddy 2 turn | Current S2/S4 bridge | auto (host) | Opposing wheels and turn plan | No straight flag, no forward distance. |
| S4-U193 | Stale wheel sample | Current S2/S4 bridge | auto (host) | Age 50 ms | Motion and distance invalid. |
| S4-U194 | Busy status copy | Current S2/S4 bridge | auto (host) | Producer returns BUSY | Motion and distance invalid. |
| S4-U195 | Clock failure | Current S2/S4 bridge | auto (host) | Producer clock unavailable | Motion and distance invalid. |
| S4-U196 | Future source timestamp | Current S2/S4 bridge | auto (host) | Wheel sample after source-now | Motion and distance invalid. |
| S4-U197 | Unequal target speeds | Current S2/S4 bridge | auto (host) | Differential target | Motion available but not straight. |
| S4-U198 | Unequal measured speeds | Current S2/S4 bridge | auto (host) | Differential wheel feedback | Motion available but not straight. |
| S4-U199 | Trust revocation API | Current S4 runtime | auto (host) | Assert, revoke and call deprecated scalar API | Explicit revocation succeeds; scalar valid input rejected. |
| S4-U202 | Short I2C read | Current SDK transport | auto (host) | [transport_main.c](host/transport_main.c), fake SDK | Short read propagates IO_ERROR; register write/read use the configured 2 ms deadline. |
| S4-U203 | Initialization write timeout | Current SDK transport | auto (host) | [transport_main.c](host/transport_main.c), fake SDK | SDK write failure prevents open; configured 2 ms timeout is passed. |
| S4-U205 | Flat acceleration then braking after valid hump | Current height guard | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c); eight 20 ms intervals each at +0.2 g and -0.2 g | False ascent reaches PEAK but is invalid; prior 10 mm synthetic maximum stays valid through return to MONITORING. |
| S4-U206 | Separate height enable gate | Current height guard | auto (host, target) | [buddy4_cases.c](host/buddy4_cases.c); default config, valid forward/trusted synthetic input | No valid maximum while height calibration remains false. |
| S4-U207 | Packed status round trip | Tribunal 1 minimum-bit representation | auto (host, target) | Encode and decode maximum bounded fields and all validity flags | Every public field survives unchanged. |
| S4-U208 | Status representation size | Tribunal 1 minimum-bit representation | auto (host, target) | Compile-time 32-byte checks plus historical host-size comparison | Core and explicit packed status are 32 bytes; host payload falls from 52 bytes. |
| S4-U209 | Packed status validation | Tribunal 1 minimum-bit representation | auto (host, target) | Inject reserved bits, invalid phase and null inputs | Decoder/encoder reject invalid representations without publishing them. |
| S4-U170 | Inspect exact breakout, supply and bus pull-ups; verify GP16/17 and axis mapping with power safely isolated | Hardware acceptance | manual (pending) | Authorized physical trial; record conditions and measured errors | Correct part, exclusive pins, common ground, compatible 3.3 V logic; save wiring record |
| S4-U171 | Stationary calibration, moved startup, known positive/negative tilt fixtures | Hardware acceptance | manual (pending) | Authorized physical trial; record conditions and measured errors | Calibration only under intended stationary conditions; measured bias/noise and tilt error within team-approved limits |
| S4-U172 | Repeated known humps with differing heights, lengths and slopes at measured speeds | Hardware acceptance | manual (pending) | Authorized physical trial; record conditions and measured errors | Correct highest physical hump, peak-time event, recorded height errors within agreed limits; invalid estimates clearly flagged |
| S4-U173 | Controlled turns with motors energized; magnetic calibration and interference checks | Hardware acceptance | manual (pending) | Authorized physical trial; record conditions and measured errors | Recorded heading/turn error and invalidity behavior within agreed limits; no gyro claim |
| S4-U174 | Restrain vehicle, inject bus disconnect/stuck bus, task load and full normal event buffer | Hardware acceptance | manual (pending) | Authorized physical trial; record conditions and measured errors | Measured sampling/jitter and independent controller/Buddy 2 fault-to-stop latency meet reviewed limits |
| S4-U175 | Authorized restrained impact and no-impact rough-terrain trials | Hardware acceptance | manual (pending) | Authorized physical trial; record conditions and measured errors | Recorded missed/false impact rate; no unsafe automatic restart; stop path preserves Buddy 2 ownership |
| S4-U200 | Flat acceleration/braking across speeds | Hardware acceptance | manual (pending) | Controlled repeat passes with raw IMU/encoder traces | No valid hump; record rejected candidates and false-valid rate |
| S4-U201 | Wheel slip, reverse and in-place turns | Hardware acceptance | manual (pending) | Restrained/controlled passes with controller revocation | No valid height and bounded revocation delay |

### Scope and limitations

Current cases compile `app_program/subsystem4/`: 54 core/driver cases also
compile into `TEST=1`; 23 runtime/bridge and 2 SDK-transport cases run in
fresh host processes.
Neither fake kernel validates SMP scheduling, electrical safety or physical
timing. Compilation is not an on-target test pass. Synthetic tolerances are
regression fixtures, not physical acceptance criteria.

The 11.187815 mm braking case now has a registered regression. The original
reported 2.313 mm input trace was unavailable; no exact reproduction is
claimed. The peak-time output estimates sensor-path rise from pitch and
forward distance; it is not a calibrated physical hump height. The
LSM303DLHC has no gyro. Height calibration is disabled by default; controller
traction assertion and physical acceptance remain integration/hardware work.
Hardware cases require separate authorization and agreed numerical limits.

## Results

Append a new row for every run; never edit previous results. No staging or
commit was authorized, so new evidence records HEAD plus the uncommitted
working tree, compiler, commands and source hashes.

### Previous legacy/monitor results (unchanged)

The following rows are preserved from the previous log. Their referenced
older evidence files were subsequently deleted at the user's direction.
They describe prior runs, not new verification; the example row is a placeholder.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| YYYY-MM-DD | S4-U01 | `abc1234` | | PASS / FAIL | `logs/YYYY-MM-DD_….txt` | |
| 2026-10-15 | S4-U003–S4-U115 | Subject `82f2745`; harness `a405c5a` + uncommitted test-folder changes | Codex | PASS, host simulation | [normal console](logs/normal.fsXov0/console.txt) | 113 tests, 2939 assertions, 0 failures; Apple Clang 21, C11, O2, warnings treated as errors. |
| 2026-10-15 | S4-U003–S4-U115 | Subject `82f2745`; same harness working tree | Codex | PASS, host simulation with ASan + UBSan | [sanitizer console](logs/sanitize.FqCkMD/console.txt) | 113 tests, 2939 assertions, 0 failures; no sanitizer diagnostics. |


### Previous current-firmware results (unchanged)

The rows retain their original commands and historical document references.
The previous BARR_REVIEW.md was deleted at the user's direction; it is not
a current compliance report.

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

### Consolidated-layout verification

Subject: `4240895a455679ad0a3996057e03723cf2e9daea` on `buddy4-fix`, plus
uncommitted test-layout, runner and path-reference changes. Production
algorithms are unchanged. Each new console records the tested source hashes.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | Initial combined host run | `4240895 + WT` | Apple Clang 21 | FAIL (legacy compile); current 51 PASS | [console](logs/2026-10-15_host_normal.GePLCD/console.txt) | Legacy sources were deleted during the run; user subsequently chose to retain only the 51 current cases. No legacy result is claimed. |
| 2026-10-15 | S4-U116–S4-U166, intermediate sanitizer run | `4240895 + WT` | Apple Clang 21 | PASS, 51 host cases | [console](logs/2026-10-15_host_sanitize.sRKlax/console.txt) | Before removing legacy runner options; ASan + UBSan, zero failures. |
| 2026-10-15 | First relocated build-runner launch | `4240895 + WT` | Codex | BLOCKED before Make | Session transcript; empty `logs/2026-10-15_build.PO7S5M/console.txt` | Sandbox denied `/dev/fd` process substitution. Replaced logging with a pipefail-preserving tee pipeline, then reran. |
| 2026-10-15 | Four integrated GNU Make profiles | `4240895 + WT` | Arm GNU 15.3.1 | PASS, compilation/link/UF2 generation | [build console](logs/2026-10-15_build.CFc75E/console.txt) | Default, TEST=1, S4_IMU=1, TEST=1 S4_IMU=1; zero warning/error diagnostics. Relocated shared test object and on-target include compile successfully; no flashing. |
| 2026-10-15 | S4-U116–S4-U166, final current-only runner | `4240895 + WT` | Apple Clang 21 | PASS, 51 host cases | [normal console](logs/2026-10-15_host_normal.EEjxut/console.txt) | Strict C99, O2; exactly 51 unique IDs, zero failures. |
| 2026-10-15 | S4-U116–S4-U166, final current-only runner | `4240895 + WT` | Apple Clang 21 | PASS, 51 host cases, ASan + UBSan | [sanitizer console](logs/2026-10-15_host_sanitize.EYKHjP/console.txt) | Exactly 51 unique IDs, zero failures; no sanitizer diagnostics. |

Exact final commands (repository root):

```sh
bash testing/subsystem4/host/run_host_tests.sh normal
bash testing/subsystem4/host/run_host_tests.sh sanitize
bash testing/subsystem4/host/run_builds.sh
bash -n testing/subsystem4/host/run_host_tests.sh testing/subsystem4/host/run_builds.sh
git diff --check
```

The build runner executes `make -s -j8`, `make -s TEST=1 -j8`,
`make -s S4_IMU=1 -j8` and `make -s TEST=1 S4_IMU=1 -j8` from `build_make/`.
The four profiles use the defaults listed above and the existing SDK at
`/Users/eddy/pico/pico-sdk`. No dependencies were installed or fetched.

Static verification: both shell scripts parse; whitespace checks pass; the
registry contains each of the 51 current IDs exactly once. All nine relocated
C/header test files match the former current suite byte-for-byte except the
three necessary relative-include path adjustments. Invalid runner mode
`replay` prints usage and exits 2. The retired directories are absent.

No firmware algorithm, calibration value or motion interface changed. No
on-target run, physical measurement, or braking-validity fix is claimed.

### Requested Buddy 4 re-verification — 2026-10-15

Tested `buddy4-fix`, HEAD `4240895a455679ad0a3996057e03723cf2e9daea`, with
the same uncommitted relocation/path changes. This verification only adds
results/evidence, not firmware changes or additional registered suite cases.
Commands are the three final commands above: host `normal`, host `sanitize`,
and `host/run_builds.sh` (the same four documented GNU Make profiles).

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S4-U116–S4-U166 | `4240895 + WT` | Apple Clang 21 | PASS, 51/51 | [normal](logs/2026-10-15_host_normal.6vHi9h/console.txt) | Strict C99/O2, all 51 unique IDs match the registry. |
| 2026-10-15 | S4-U116–S4-U166 | `4240895 + WT` | Apple Clang 21 | PASS, 51/51, ASan + UBSan | [sanitizer](logs/2026-10-15_host_sanitize.CbqPcz/console.txt) | No sanitizer diagnostics or failed cases. |
| 2026-10-15 | Four integrated profiles | `4240895 + WT` | Arm GNU 15.3.1 | PASS, compilation/link/UF2 | [build](logs/2026-10-15_build.r1rA3n/console.txt) | Default, TEST=1, S4_IMU=1, TEST=1 S4_IMU=1; zero warning/error diagnostics; no flash. |
| 2026-10-15 | AUD-05 ad-hoc braking diagnostic (outside registered suite) | `4240895 + WT` | Apple Clang 21 | FAIL: false valid maximum | [inputs and output](logs/2026-10-15_verification.y3BrBf/flat_braking_and_scope.txt) | Flat 0.2 g braking with consistent decreasing wheel distance produces 11.187815 mm, b_height_valid=true and b_highest_valid=true at PEAK. New sequence, not the original 2.313 mm fixture. |
| 2026-10-15 | Layout / syntax / integration inspection | `4240895 + WT` | Codex | Static checks pass; integration incomplete | [inspection](logs/2026-10-15_verification.y3BrBf/flat_braking_and_scope.txt) | Correct test paths; no production speed provider or status/event/fault consumer. Hardware access and magnetic validity remain disabled by default. |

Overall: **not a full functional acceptance pass**. Existing cases pass, but
the separately reproduced braking defect remains. Physical hump accuracy,
magnetic interference, bus-fault handling on hardware, SMP timing and the
controller-to-motor stop path are still unverified. No fixes were implemented.

### Buddy 4 braking-validity fix — 2026-10-15

Subject: `buddy4-fix`, HEAD `4240895a455679ad0a3996057e03723cf2e9daea`
plus uncommitted Buddy 4 firmware, test and documentation changes. The current
registry now contains **75** cases: S4-U116–S4-U166 and S4-U176–S4-U199.
The 11.187815 mm flat-braking sequence, newly preserved as S4-U176, has no
valid maximum after the fix. The reported 2.313 mm input trace was not
available, so that exact result is not claimed reproduced.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S4-U161, intermediate | `4240895 + WT` | Codex | FAIL | [console](logs/2026-10-15_host_normal.TYD4vS/console.txt) | Old scalar-speed expectation; updated for explicit trust/revocation. |
| 2026-10-15 | S4-U189, intermediate | `4240895 + WT` | Codex | FAIL | [console](logs/2026-10-15_host_normal.ea6nGb/console.txt) | Fixture helper derived speed from distance; corrected fixture distance. |
| 2026-10-15 | S4-U198, intermediate | `4240895 + WT` | Codex | FAIL | [console](logs/2026-10-15_host_normal.8rCPYW/console.txt) | Bridge straight flag now checks measured wheel difference. |
| 2026-10-15 | S4-U116–S4-U166, U176–U199 | `4240895 + WT` | Apple Clang 21 | PASS, 75/75 host | [normal](logs/2026-10-15_host_normal.j5sffE/console.txt) | Strict C99, O2, 75 distinct IDs. Source hashes in console; before final source-domain bound adjustment. |
| 2026-10-15 | S4-U116–S4-U166, U176–U199 | `4240895 + WT` | Apple Clang 21 | PASS, 75/75, ASan + UBSan | [sanitizer](logs/2026-10-15_host_sanitize.8Pwfxy/console.txt) | No sanitizer diagnostics; before final source-domain bound adjustment. |
| 2026-10-15 | Four integrated Make profiles | `4240895 + WT` | Arm GNU 15.3.1 | PASS, compile/link/UF2 | [build](logs/2026-10-15_build.N8cGPP/console.txt) | Default, TEST=1, S4_IMU=1, both; no flash. Before final source-domain bound adjustment. |

Final rerun evidence is appended below after verification. Host cases use a
synthetic envelope; the production default remains uncalibrated and height
invalid. No Pico/IMU/wheel hardware was accessed. Physical height error,
traction policy, magnetic interference, bus-fault latency and SMP response
remain unmeasured. The read-only Buddy 2 status bridge and controller trust
interface need owner-level integration review before mission use.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S4-U116–S4-U166, U176–U199 | `4240895 + WT` | Apple Clang 21 | PASS, 75/75 host | [normal](logs/2026-10-15_host_normal.6MJ71W/console.txt) | Final source-domain bounds and persistence guard; strict C99/O2, zero failures, 75 distinct IDs. |
| 2026-10-15 | S4-U116–S4-U166, U176–U199 | `4240895 + WT` | Apple Clang 21 | PASS, 75/75, ASan + UBSan | [sanitizer](logs/2026-10-15_host_sanitize.JCocgn/console.txt) | Final sources; no sanitizer diagnostics. |
| 2026-10-15 | Four integrated Make profiles | `4240895 + WT` | Arm GNU 15.3.1 | PASS, compile/link/UF2 | [build](logs/2026-10-15_build.69j3MR/console.txt) | Final sources; default, TEST=1, S4_IMU=1, both; no flash. |

### Requested recheck — 2026-10-15

Tested branch `buddy4-fix`, HEAD
`4240895a455679ad0a3996057e03723cf2e9daea` plus the uncommitted
working tree shown in each console. Commands from the repository root:

```sh
bash testing/subsystem4/host/run_host_tests.sh normal
bash testing/subsystem4/host/run_host_tests.sh sanitize
bash testing/subsystem4/host/run_builds.sh
```

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S4-U116–S4-U166, U176–U199 | `4240895 + WT` | Apple Clang 21 | PASS, 75/75 host | [normal](logs/2026-10-15_host_normal.3WGSPL/console.txt) | Strict C99/O2, zero failed cases. |
| 2026-10-15 | S4-U116–S4-U166, U176–U199 | `4240895 + WT` | Apple Clang 21 | PASS, 75/75, ASan + UBSan | [sanitizer](logs/2026-10-15_host_sanitize.SAl1Ih/console.txt) | Zero failed cases or sanitizer diagnostics. |
| 2026-10-15 | Four integrated Make profiles | `4240895 + WT` | Arm GNU 15.3.1 | PASS, compile/link/UF2 | [build](logs/2026-10-15_build.95eC9N/console.txt) | Default, TEST=1, S4_IMU=1 and both. No flashing or hardware test. |

Static check: runner shell syntax and diff whitespace pass. The integrated
source wires S4 init/start in the template order. No application caller of
`imu_runtime_set_motion_trust`, `imu_runtime_get_event`,
`imu_runtime_get_status` or `imu_runtime_wait_fault` exists yet. Default
`S4_IMU_ENABLE=0` and `S4_HEIGHT_CALIBRATED=false` leave physical sensing and
calibrated height reporting inactive; passing builds do not establish mission
operation, measured height accuracy, fault-to-motor-stop latency or SMP timing.

### Braking regression and enablement review — 2026-10-15

Tested branch `buddy4-fix`, HEAD
`f3e77ffb5fc0d1ddeb7b963bed939fa926db89fc` plus the uncommitted
Buddy 4 test/documentation changes. The current registry has 79 cases. The
new cases were defined in this registry and rerun in the final logs below.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S4-U116–U166, U176–U199, U202–U203, U205–U206 | `f3e77ff + WT` | Apple Clang 21 | PASS, 79/79 host | [normal](logs/2026-10-15_host_normal.eOBGWx/console.txt) | Strict C99/O2, 79 distinct IDs; S4-U176 braking and S4-U205 prior-max regression both pass. |
| 2026-10-15 | Same 79 cases | `f3e77ff + WT` | Apple Clang 21 | PASS, 79/79, ASan + UBSan | [sanitizer](logs/2026-10-15_host_sanitize.czAhGd/console.txt) | No failed cases or sanitizer diagnostics. |
| 2026-10-15 | Four integrated GNU Make profiles | `f3e77ff + WT` | Arm GNU 15.3.1 | PASS, compile/link/UF2 | [build](logs/2026-10-15_build.EzwBWB/console.txt) | Default, TEST=1, S4_IMU=1 and both; no warnings/errors, no flashing. |

Regression comparison: the [pre-fix diagnostic](logs/2026-10-15_verification.y3BrBf/flat_braking_and_scope.txt)
used ten 20 ms flat-ground braking samples at -0.2 g and reported a false
11.187815 mm valid maximum. Current S4-U176 retains that accelerometer and
distance sequence, adds the independent signed-motion evidence required by
the new core, and asserts PEAK with no valid height/maximum and a DYNAMIC
reason. S4-U205 first establishes a valid 10 mm synthetic maximum, then
runs eight +0.2 g acceleration samples and eight -0.2 g braking samples;
the braking false ascent reaches PEAK but cannot replace that maximum through
return to MONITORING. The earlier reported 2.313 mm inputs were unavailable,
so that exact case cannot be claimed reproduced. These results hold for the
synthetic guard envelope in the tests, not for every speed/surface/vehicle.

S4-U202/203 use an SDK double to verify the application wrapper passes its
configured 2 ms deadline/timeout and propagates short-read/write failures.
Earlier S4-U155–159 exercise sensor-open/read faults, stale samples and full
ordinary event queue; the latched fault flag stays readable when the queue
cannot accept an event. S4-U180–184 and U190–199 exercise stale/reverse/turn
motion and explicit trust revocation. This is software fault-delivery evidence,
not measured physical stuck-bus or stop latency.

Static inspection confirms GP16/GP17 and I2C0 are assigned to S4 in `TEAM.md`,
the kernel I2C driver is disabled, and `drv_imu.c` rejects simultaneous
kernel I2C ownership. The shared Vehicle Controller reads S4 status and its
independent fault latch, then requests Buddy 2 emergency stop. It currently
does not assert traction trust or consume ordinary hump events. Actual sensor,
wiring, pull-ups, mounting, calibration, slip discrimination, physical height
error, SMP timing and fault-to-motor-stop latency remain unverified. Both
`S4_MAG_CALIBRATED` and `S4_HEIGHT_CALIBRATED` remain false; no hardware was
accessed or enabled.

### Tribunal 1 minimum-bit representation — 2026-10-21

Subject: `buddy4-afterTribunal`, HEAD
`d4c91f271c9801802e3994967c96e9ec8fcba38c` plus the Buddy 4 firmware, test
and documentation changes recorded in each console. The registry now has 82
cases. S4-U207–U209 cover round-trip correctness, the 32-byte representation
and rejection of invalid packed states.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-21 | S4-U116–U166, U176–U199, U202–U203, U205–U209 | `d4c91f2 + WT` | Apple Clang 21 | PASS, 82/82 host | [normal](logs/2026-10-21_host_normal.9WshOY/console.txt) | Strict C99/O2; zero failed cases. |
| 2026-10-21 | Same 82 cases | `d4c91f2 + WT` | Apple Clang 21 | PASS, 82/82, ASan + UBSan | [sanitizer](logs/2026-10-21_host_sanitize.6HWd3X/console.txt) | No sanitizer diagnostics. |
| 2026-10-21 | Four integrated GNU Make profiles | `d4c91f2 + WT` | Arm GNU 15.3.1 | PASS, compile/link/UF2 | [build](logs/2026-10-21_build.YKRfk8/console.txt) | Default, TEST=1, S4_IMU=1 and both; no flashing. |

The shared ARM integration compiler check also passed all four single/SMP and
normal/TEST combinations (44 objects total). Its host runner was unavailable
on macOS because it hard-codes a Windows GCC path and requires GCC
`-fanalyzer`; shared test infrastructure was not changed. No Pico was flashed,
and physical calibration, hump accuracy, magnetic interference, fault-to-stop
latency and 20 ms scheduling remain unverified.

### Buddy 4 checklist and cross-subsystem build — 2026-10-24

Tested `buddy4-afterTribunal` at HEAD `730056102a6d60abb22bcb73345393a149692bd4`
plus the uncommitted Buddy 4 monitor-lock check. The 82 current host cases do
not execute the retained template monitor; its new lock-failure path was
inspected and compiled in all integrated profiles, not exercised on target.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-24 | S4-U116–U166, U176–U199, U202–U203, U205–U209 | `7300561 + WT` | Codex / Apple Clang 21 | PASS, 82/82 host | [normal](logs/2026-10-24_host_normal.55yVJn/console.txt) | Strict C99/O2; no failures. |
| 2026-10-24 | Same 82 cases | `7300561 + WT` | Codex / Apple Clang 21 | PASS, 82/82 host with ASan + UBSan | [sanitizer](logs/2026-10-24_host_sanitize.Ex37rH/console.txt) | No sanitizer diagnostics. |
| 2026-10-24 | Integrated Make profiles | `7300561 + WT` | Codex / Arm GNU 15.3.1 | PASS, compile/link/UF2 | [build](logs/2026-10-24_build.GiW992/console.txt) | Default, TEST=1, S4_IMU=1 and both; no errors or warnings, no flash. |

Checklist inspection: S4 uses no S3 navigation queue or `s3_submit()` call,
and reads the retained S3 `app_stats` only after confirming `mtx_stats` was
locked. The lock is released before printing. The compact IMU status remains
private to S4 and does not change S3's public API. Its minimum-bit table,
before/after RP2040 and host sizes, and packed-state correctness tests are in
`docs/buddy4/DATA_REPRESENTATION.md` and S4-U207–U209. The retained template
monitor still controls Buddy 1's demonstration LED task through IF-05; this
existing cross-buddy behaviour was not changed or hardware-tested. Physical
IMU enablement and calibration gates remain pending.

## Robot raw diagnostic copy definitions 2026-10-28

| ID | Method | Expected result |
|---|---|---|
| S4-U210 | APP_ROBOT=1 runtime fake | Null arguments rejected; unobserved raw sample has result UINT32_MAX; copying performs no additional sensor read |
| S4-U211 | Same, mutex failure | Return E_TMOUT and preserve caller storage; no extra sensor read |

Included by `testing/integration/verify_integration.py --robot`. Target timing and actual IMU readings remain NOT RUN.

S4-U212: run the actual IMU owner with the existing sensor fake through a transport failure. Expected: diagnostics retain the last successful acceleration/time, report the latest IO error, and perform no additional sensor read. Included in the robot host suite.


### Software verification run 2026-10-28 (uncommitted production commissioning)

HEAD `993927ce2a0a258f7dc7ab91838194a22d45eda0` plus the approved dirty working
tree. Final commands, scope, source/artifact hashes, external raw evidence paths
and the complete changed-file list are recorded in
[`docs/commissioning.md`](../../docs/commissioning.md#software-verification-and-delivery-2026-10-28).
No physical hardware test, firmware flash or real MQTT session was performed.

S4-U210..212 pass in both robot host profiles. Existing core/runtime/transport functional checks pass with the pre-existing conversion/type-limit warning categories nonfatal and visible. Strict selected imu_runtime.c ARM compilation passes; no clean strict whole-S4 or physical calibration claim.

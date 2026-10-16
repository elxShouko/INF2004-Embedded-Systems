# Integration — test log

Tests that two or more subsystems work together. There is one test (or more) for every interface listed in TEAM.md section 5. Both owners agree on the test and sign off the result.

See [testing conventions](../README.md) for how to run tests and record results.

## Test cases

Define a test here before running it. Test IDs never change.

| ID | What is tested | Requirement / interface | Method | Steps / how | Expected result |
|---|---|---|---|---|---|
| INT-01 | samples flow from S2 to S3 (≥ 5 in 2 s) | IF-01, IF-02 | auto | `subsystem3_test.c` | `[TEST] INT-01 PASS` |
| INT-02 | summary lines flow from S3 to S4 (≥ 2 in 3.5 s) | IF-03 | auto | `subsystem4_test.c` | `[TEST] INT-02 PASS` |

*Method:* **auto** = `TEST_CHECK` in code, result printed on the console;
**manual** = a person observes the result (LED, scope, meter, phone app…).

## Results

Add a new row for every run. Never edit or delete old rows.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| YYYY-MM-DD | INT-01 | `abc1234` | | PASS / FAIL | `logs/YYYY-MM-DD_….txt` | |

## Phase 2 shared integration — definitions before execution

Host cases use actual shared controller/startup/dispatcher and selected real
S1/S2/S3/S4/S5 production algorithms with kernel, public-provider and MMIO fakes.
Each case runs in a fresh process. These are not target or physical tests.

| ID | What is tested | Method | Expected result |
|---|---|---|---|
| INT-03 | startup keeps motors unconfigured and disables normal synthetic telemetry | test_vehicle.c, case 3; host | All assertions true; one matching PASS ID |
| INT-04 | init failure prevents ALL subsystem starts | test_vehicle.c, case 4; host | All assertions true; one matching PASS ID |
| INT-05 | shared task helper propagates start error and deletes dormant task | test_vehicle.c, case 5; host | All assertions true; one matching PASS ID |
| INT-06 | unsupported S5/bounded S3 absence cannot enable motion | test_vehicle.c, case 6; host | All assertions true; one matching PASS ID |
| INT-07 | dedicated remote emergency precedes ordinary getters | test_vehicle.c, case 7; host | All assertions true; one matching PASS ID |
| INT-08 | clock failure propagates emergency before other sensing | test_vehicle.c, case 8; host | All assertions true; one matching PASS ID |
| INT-09 | controller delay failure publishes fault and exits | test_vehicle.c, case 9; host | All assertions true; one matching PASS ID |
| INT-10 | S3 rejected request propagates emergency | test_vehicle.c, case 10; host | All assertions true; one matching PASS ID |
| INT-11 | S1 command + S3 steering reaches actual S2 command acceptance | test_vehicle.c, case 11; host | All assertions true; one matching PASS ID |
| INT-12 | BUSY/queued intake retries same ID and timestamp until receipt | test_vehicle.c, case 12; host | All assertions true; one matching PASS ID |
| INT-13 | owner rejection faults without optimistic motion acceptance | test_vehicle.c, case 13; host | All assertions true; one matching PASS ID |
| INT-14 | continuous refresh retains ID and S2 sampling deadline | test_vehicle.c, case 14; host | All assertions true; one matching PASS ID |
| INT-15 | pause uses STOP then ACK then PAUSED without refresh | test_vehicle.c, case 15; host | All assertions true; one matching PASS ID |
| INT-16 | provider loss during live motion faults | test_vehicle.c, case 16; host | All assertions true; one matching PASS ID |
| INT-17 | real S3 mapping requests LEFT with correlated event ID | test_vehicle.c, case 17; host | All assertions true; one matching PASS ID |
| INT-18 | finite completion ACK precedes S3 manoeuvre-done | test_vehicle.c, case 18; host | All assertions true; one matching PASS ID |
| INT-19 | emergency preempts finite manoeuvre | test_vehicle.c, case 19; host | All assertions true; one matching PASS ID |
| INT-20 | real S5 unverified clearance yields no bypass command | test_vehicle.c, case 20; host | All assertions true; one matching PASS ID |
| INT-21 | raw and operating clocks are never treated as same origin | test_vehicle.c, case 21; host | All assertions true; one matching PASS ID |
| INT-22 | mandatory telemetry absence suppresses fake full frames | test_vehicle.c, case 22; host | All assertions true; one matching PASS ID |
| INT-23 | GPIO wrong-core and overlap rejection | test_vehicle.c, case 23; host | All assertions true; one matching PASS ID |
| INT-24 | owned edge acknowledgement preserves foreign status bits | test_vehicle.c, case 24; host | All assertions true; one matching PASS ID |
| INT-25 | coherent GPIO edge reaches real S2 quadrature decoder | test_vehicle.c, case 25; host | All assertions true; one matching PASS ID |
| INT-26 | release prevents later callbacks and rejects double release | test_vehicle.c, case 26; host | All assertions true; one matching PASS ID |
| INT-27 | coalesced opposite edges latch fault, no healthy decoder feed | test_vehicle.c, case 27; host | All assertions true; one matching PASS ID |
| INT-28 | two GPIO clients receive only their own masks | test_vehicle.c, case 28; host | All assertions true; one matching PASS ID |
| INT-29 | IRQ install refusal leaves no reservation | test_vehicle.c, case 29; host | All assertions true; one matching PASS ID |
| INT-30 | invalid mission enums fail closed | test_vehicle.c, case 30; host | All assertions true; one matching PASS ID |
| INT-31 | controller task creation failure rolls back mutex | test_vehicle.c, case 31; host | All assertions true; one matching PASS ID |
| INT-32 | uncalibrated mission configuration rejected | test_vehicle.c, case 32; host | All assertions true; one matching PASS ID |
| INT-33 | actual IMU impact event propagates emergency | test_vehicle.c, case 33; host | All assertions true; one matching PASS ID |
| INT-34 | readiness never enables motors without explicit START | test_vehicle.c, case 34; host | All assertions true; one matching PASS ID |
| INT-35 | cancelled finite manoeuvre faults rather than reporting navigation success | test_vehicle.c, case 35; host | ESTOP; no ACK or success |
| INT-36 | invalid line cannot allocate motion IDs | test_vehicle.c, case 36; host | no command, next/active IDs unchanged |
| INT-37 | each new barcode clears previous line-recovery evidence | test_vehicle.c, case 37; host | previous LINE_FOUND latch cleared |
| INT-38 | edge arriving during GPIO snapshot cannot be silently acknowledged using old levels | test_vehicle.c, case 38; host | fault; no decoder callback |
| INT-39 | signed wheel odometry becomes relative forward path; duplicate sample never refreshed | test_vehicle.c, case 39; host | 1000 um; source-aged raw timestamp; duplicate rejected |
| INT-40 | odometry discontinuity/huge signed delta cannot overflow or imply forward travel | test_vehicle.c, case 40; host | cumulative path unchanged; forward validity revoked |
| INT-41 | enabled unregistered GPIO interrupt source cannot starve emergency handling | test_vehicle.c, case 41; host | fault before callbacks; foreign bits preserved; native adapter disables bank NVIC |
| INT-42 | foreign active motion faults even before configuration or with readiness blockers | test_vehicle.c, case 42; host | ESTOP takes precedence over UNCONFIGURED/BLOCKED |
| INT-43 | turn intake retry cannot override PAUSE or near obstacle | test_vehicle.c, case 43; host | ESTOP, no retry |
| INT-44 | provider samples newer than early loop clocks use post-read clocks | actual runtime with API fakes, case 44 | S2/IMU fresh; missing S3/S5 still block; no enable |
| INT-45 | subsystem start failure stops later starts and posts emergency | actual usermain, case 45 | 3 start calls; emergency; no enable |
| INT-46 | terminal acknowledgement cannot bypass transition deadline | actual mission policy, case 46 | deadline faults before repeated ACK |
| INT-47 | PAUSE/RESUME retains line-recovery obligation | actual mission policy, case 47 | recovery mode; no command before fresh LINE_FOUND |

## Phase 2 verification results (2026-10-15)

Base: local main `9c9ff747613eea900c0a25a590855f2e66cf0af4`.
Branch: `codex/phase2-system-integration`; **uncommitted working-tree evidence**,
not a tested integration commit. No fetch, commit, push, merge or hardware run.
This earlier 40-case run is superseded by the commit review below. Its source
hashes/logs retain their original `phase2_20261015_*release*` names inside the
development evidence archive. Source hashes were recorded in
[host metadata](logs/phase2_20261015_development_evidence.zip) and
[ARM metadata](logs/phase2_20261015_development_evidence.zip).

Commands, from repository root, using the installed Python executable:

```text
python -B testing/integration/verify_integration.py --label phase2_20261015_release
python -B testing/integration/verify_integration.py --label phase2_20261015_arm_release --arm
```

| Scope | Result | Evidence / limitation |
|---|---|---|
| INT-03�INT-42 default host | 40/40 PASS | [default log](logs/phase2_20261015_development_evidence.zip); GCC 14.2, C99, strict warnings and `-fanalyzer` |
| INT-03�INT-42 optimized host | 40/40 PASS | [optimized log](logs/phase2_20261015_development_evidence.zip); additionally `-O2 -fshort-enums` |
| Shared ARM objects | 16/16 PASS | [ARM log](logs/phase2_20261015_development_evidence.zip); GCC 10.3, unchanged archived Make recipe, SMP=0/1 x APP_TEST=0/1, four shared C files |
| INT-01/INT-02 professor demos | NOT RUN in this pass | Existing calls/tests preserved; old placeholder row is not evidence |
| Full firmware linking / UF2 | NOT RUN in this pass | Required SDK 2.2.0 absent from checked environment/locations; discovered 1.5.1; no SDK/build patches or installation |
| On-target / physical / network | NOT RUN | No flash, motor enable, physical sensors or live MQTT |

No warnings or analyzer findings in the new shared production code/tests.
Existing S1 codec conversion warnings remain visible at `s1_codec.c:217,233,271`;
only that provider object permits those two warning classes without `-Werror`.
It was not rewritten. All other host objects and all four new/shared ARM objects
retain strict warnings-as-errors. SDK/kernel headers are system includes for
ARM warning isolation, not source modifications. Native vector installation,
MMIO ordering and ISR timing are inspected/compiled, **not hardware-tested**.
ARM per-function static stack estimates include vehicle task 832 bytes, mission
step 136 bytes and dispatcher 96 bytes; these are not call-chain or interrupt
high-water measurements. The task reserves 8192 bytes pending target measurement.

Self-review corrected terminal ACK/STOP precedence, cancellation of finite turns,
consumption of IDs before valid line admission, old recovery evidence reuse,
GPIO pending/levels snapshot races, unregistered enabled IRQ starvation and
foreign active motion hidden by missing readiness/configuration.
Invalid source enums/faults fail closed. The test task-start fixture uses a
valid ABI callback; kernel/MMIO/public-provider fakes are explicit. Production
S1 codec, S2 executive/estimation/control, S3 classification/mapping, S4 impact
logic and S5 clearance rejection are actually linked. Simulated DONE/status
fixtures do not establish real turning or sensor performance.

Current non-actuating integration is **PASS WITH CONDITIONS for review**:
bounded S3 status, live S5 sensing/planning, S1 emergency publication and telemetry
contracts remain blockers. No default calibration, bypass, traction assertion,
fake clear obstacle profile or fabricated full telemetry is introduced.
[TEAM.md section 7](../../TEAM.md#7-phase-2-shared-integration-2026-10-15)
records ownership, units/clock domains, handshakes, readiness, lifecycle limits
and provider-specific corrections. A fault requires restart; shared task startup
and GPIO activation must still be reviewed on the target before powered use.

Evidence hygiene: intermediate attempts (including the initial compile failure,
failed IMU fixture and superseded passing runs) are retained byte-for-byte in
[development evidence archive](logs/phase2_20261015_development_evidence.zip),
with SHA-256 manifests inside. Final logs are kept separately. No committed
historical evidence was deleted. Temporary compiler objects, executables and
`.su` files were confined to ignored `testing/integration/build/` run directories
and automatically removed. No new `.gitignore` or Makefile changes were needed.


### Final independent commit review

Authorized final review on the same main base, before staging. Confirmed shared
corrections: pending-turn retries no longer precede PAUSE/obstacle safety;
clock snapshots are refreshed after provider reads; ACK/recovery waits are
bounded; recovery cannot be forgotten by PAUSE/RESUME; fatal S3 manoeuvre-done
submission errors now request emergency. INT-43 through INT-47 cover the new
ordering/state regressions and a missing partial-start failure case. Existing
40 cases are preserved. No buddy implementation, template/kernel or Makefile edit.

Commands (same installed Python, tool paths and profiles as above):

```text
python -B testing/integration/verify_integration.py --label phase2_commit_final
python -B testing/integration/verify_integration.py --label phase2_commit_final_arm --arm
```

| Verification | Actual result | Evidence |
|---|---|---|
| INT-03 to INT-47, default | 45/45 PASS; strict warnings + GCC analyzer | [host log](logs/phase2_commit_final_default.log) |
| INT-03 to INT-47, optimized | 45/45 PASS; O2/short enums, same checks | [optimized log](logs/phase2_commit_final_optimized.log) |
| Shared ARM objects | 16/16 PASS; SMP=0/1 x APP_TEST=0/1 | [ARM log](logs/phase2_commit_final_arm_arm.log) |
| Exact source association | Both metadata sets match all 113 recorded inputs | [host metadata](logs/phase2_commit_final_metadata.json), [ARM metadata](logs/phase2_commit_final_arm_metadata.json) |

These are pre-commit working-tree tests. The eventual commit is associated by
matching raw compiler-input hashes and Git-normalized blob IDs, not by
relabelling earlier HEAD as a tested
integration commit. Previous 40-case and superseded review evidence is retained in the hash-verified
archive. INT-01/02, full linking, on-target, network and physical tests remain
unrun. SDK 2.2.0 remains missing in the checked installation/environment; no
installation or build patches were attempted. S1 conversion warnings remain
unchanged. Current provider interlocks still prevent all mission movement.

Final self-review: no remaining confirmed critical/high defect in the reviewed
shared paths; PASS WITH CONDITIONS for a non-actuating foundation PR. Native
IRQ installation/affinity, RTOS stop latency, stack high-water and full-profile
resource consumption require target/link verification. S1 safety publication,
bounded S3 status, live S5 APIs, telemetry agreement and physical calibration
remain prerequisites to completing integration or enabling motors.

Git has `core.autocrlf=true`. Final metadata records both raw SHA-256 inputs
and `git_blob` IDs computed through Git's configured normalization. Before
commit, all 113 recorded blob IDs are checked against the index, avoiding a
false mismatch or unsupported claim about CRLF-to-LF conversion. The initial
staged snapshot and earlier evidence were reconciled without losing history.

### Final changed-file inventory and evidence integrity

Explicit commit inventory (no generated objects/binaries, buddy implementation,
professor build/kernel or unrelated files):

- `TEAM.md`
- `app_program/app_main.c`
- `app_program/gpio_bank.c`
- `app_program/gpio_bank.h`
- `app_program/vehicle.c`
- `app_program/vehicle.h`
- `app_program/vehicle_rt.c`
- `app_program/vehicle_rt.h`
- `testing/integration/TEST_LOG.md`
- `testing/integration/logs/phase2_20261015_development_evidence.zip`
- `testing/integration/logs/phase2_commit_final_arm_arm.log`
- `testing/integration/logs/phase2_commit_final_arm_metadata.json`
- `testing/integration/logs/phase2_commit_final_default.log`
- `testing/integration/logs/phase2_commit_final_metadata.json`
- `testing/integration/logs/phase2_commit_final_optimized.log`
- `testing/integration/test_ports.h`
- `testing/integration/test_vehicle.c`
- `testing/integration/verify_integration.py`

Evidence hashes below describe the original working-tree bytes. Git blob IDs
bind their normalized text contents to this commit; the binary history archive
retains original bytes and embedded hashes without line-ending conversion.

| Evidence | SHA-256 (working bytes) | Git blob ID |
|---|---|---|
| `phase2_20261015_development_evidence.zip` | `fb9f9db09eb0f80a3069dbc8f534a27c7d85db34d61bc099b56eec849a0fb2bb` | `edc896f24d98e02179d2ecaf62bd692375364fc3` |
| `phase2_commit_final_arm_arm.log` | `86afcff2fb73f1afa15c7be557a7fb687f1b50346545737087d91bf9c61ceb25` | `e022e871b127a1d19c1cbcd8778914547361d8ee` |
| `phase2_commit_final_arm_metadata.json` | `98f8978db8dc88475b8be89c80b24b432638c673b999c8536601f065408b3eef` | `2d688c14f308a207e3b4bd65f5854f9ee5b3c400` |
| `phase2_commit_final_default.log` | `62c76deb4b1c86f0891fad6fdd153587f05041107eddcb48ee95ba02a6ec6c61` | `cdf948b74db72ff5d8070f3ca59da38edd2a0358` |
| `phase2_commit_final_metadata.json` | `ca8769a902199b226a20913daec747ee0b84b380a7126d27fb25def3c2903862` | `f615ff3f743b9f6d8508e4793a095462def5346f` |
| `phase2_commit_final_optimized.log` | `acac1e3e8f4e473e1c927c0497f20d9b1d020e9a82d3e6cabbf283103de5b063` | `f811692fc62213e41e9ae7a8ecc34502b9b76008` |


## Final software integration preparation � 2026-10-15

Starting clean local and live GitHub main: `5c08a5c76de39ffe18d9720b40d505a592508230`.
Work is uncommitted on `codex/final-software-integration`. Exact compiler inputs,
commands, results and source hashes are in `logs/software_preparation_evidence.zip`
(manifest inside). No target execution, flashing or physical validation occurred.
Earlier result records above remain historical; this section supersedes their
provider-blocker descriptions, not their measured outcomes.

### New integration definitions

| ID | Production behaviour and host method | Expected |
|---|---|---|
| INT-48 | Owner task sees persistent S1 latch with no pulse or queued command | FAULT, estop before configuration, no enable |
| INT-49 | Owner copies calibrated S3 raw/filtered status and original time | Line blocker clears; missing obstacle still blocks movement |
| INT-50 | S3 status getter is busy | Line unavailable; no enable |
| INT-51 | S3 copied status reports driver fault | Fault preempts configuration and movement |
| INT-52 | Shared task start fails once, retries, then duplicate start | Original E_SYS, E_OK retry, E_OBJ duplicate |
| INT-53 | S3 status is coherent but too old | Line blocker remains; no timestamp renewal |
| INT-54 | S4 transition queue contains impact/fault | Fault and estop independent of current status |
| INT-55 | S4 transition read returns kernel error | Fault and estop; no configuration |
| INT-56 | S4 has sustained transition backlog | Exactly four reads/cycle; no starvation or motion with missing S5 |

Tests use real `vehicle_rt.c`, mission policy and GPIO dispatcher with boundary
fakes; they do not prove native SMP scheduling or sensor correctness. Existing
INT-03..47 cases remain. Four scratch-source mutations must fail INT-48/49/52/54
when latch polling, line collection, failed-start rollback or event collection
is deliberately removed. Existing S2 feedback/mutation tests remain unchanged.

### Current ownership and remaining blockers

- **Shared corrections:** connect S1 latch/link, bounded S3 status (calibration,
  raw/filter validity, timestamp, mode/state and pending ID), actual S2 counts,
  bounded S4 event drain and fail-safe fault handling; restore failed task-start
  state. No subsystem implementation, Makefile, kernel or device source changed.
- **Buddy 1:** investigate U14b/U14d/U16b threaded failures recorded in this run.
  U14b reported zero torn snapshots but exhausted its reader budget; the label
  alone does not establish a data race. U14d's final totals and U16b's compound
  assertion failed; isolate scheduling/fake state before claiming a production
  cause. Serial logic passing does not cancel these failures. Agree signed
  int64 counts, sample time/generation, independent validity and IR bit mapping
  before changing `s1_codec_frame_t` or enabling full real telemetry.
- **Buddy 2:** production feedback and algorithms pass host regressions; physical
  wiring, polarity, resolution, geometry, gains, reversal and stop latency remain.
- **Buddy 3:** bounded getter now exists and is used. All six functional suites
  pass. Repository style checker rejects checkout CRLF bytes; identical disposable
  copies normalized only to LF pass. Recommend owner making checker line-ending
  aware or agreeing shared checkout settings; no teammate files were changed.
  A=straight/B=left/C=right/D=U-turn is the actual provider default; course mapping
  must be confirmed rather than inferred from the write-up's example mapping.
- **Buddy 4:** S4_IMU=0 default; mounting/bus and magnetic/height calibration remain
  explicitly unverified. Controller never fabricates traction trust or height.
- **Buddy 5:** `subsystem5.c:15` start returns E_NOSPT. New policy has no RTOS
  acquisition owner or copied live status API; header explicitly requires single
  task ownership. Need sensor/servo drivers, confirmed servo pin and cooperative
  GP6 dispatcher subscription on core 1. Planner remains advisory/non-executable
  without swept-path clearance evidence. Shared scan/bypass adapter is blocked
  until those owner-provided APIs exist; no fake clear-path status was inserted.
- **Shared mission:** obstacle bypass/moving line search, full telemetry and
  finish-marker semantics remain incomplete. No setup caller/default calibration;
  autonomous mission stays disabled. Readiness must distinguish stationary scans
  from continuous forward protection; no circular startup dependency is assumed.
- **Environment:** intended USB/Wi-Fi, TEST=1, MQTT and S4_IMU profiles fail with
  missing `pico.h` under installed SDK 1.5.1 versus Make's SDK 2.x include layout.
  Make does not enforce exactly 2.2.0. Obtain an approved compatible SDK with
  TinyUSB/cyw43/lwIP submodules and local credentials; rerun full intended build.
  Do not patch professor compatibility code to force this older installation.

### Controlled hardware session gates (not yet verified)

Order: verify wiring/power with motor and servo supplies disconnected; sensor-only
USB bring-up; wheels lifted motion tests; controlled low-speed floor tests; then
mission trials after all software/provider blockers close. Use USB console,
CRASH_REPORT=0, kernel I2C disabled. Confirm GP28 VBAT jumper disconnected and
GP8/9 not claimed by I2C. Never flash the UART evidence profile with encoders wired.

| Owner | Safe bring-up, required measurements and acceptance |
|---|---|
| Buddy 1 | Radio/broker only first; measure emergency delivery under full queues and telemetry load, reconnect/heartbeat/telemetry periods. Meet documented <=50 ms command delivery, 100 ms +/-10 ms telemetry and 1000 ms +/-50 ms heartbeat; confirm real validity fields after contract agreement. |
| Buddy 2 | Verify motor GP8/9 and GP10/11, encoder GP7/28 and GP0/1, 3.3 V electrical suitability; measure counts/revolution and both polarities with hand rotation. Wheels lifted: confirm low/low brake and high/high coast, startup non-drive, bounded reversal and estop. Measure wheel geometry, speed/distance, tune PID and turn compensation; establish and record safe speed, timing, stopping and accuracy limits before floor motion. |
| Buddy 3 | Verify GP2/4/27, polarities and calibration on actual black/white track; record stable/raw agreement, sample jitter and barcode pass rate over approved speeds. No stale samples may refresh motion; verify mapping, completion IDs and line recovery before motor coupling. |
| Buddy 4 | Verify GP16/17 I2C pull-ups and sensor identity with wheels stationary; confirm body axes and stationary calibration. Record flat acceleration/braking controls, impact and known-height hump trials; set trust/height thresholds only with measurements and reject invalid height/heading. |
| Buddy 5 | Confirm HC-SR04 supply suitability/ECHO voltage before connecting RP2040; confirm servo pin, supply and slice (GP15 provisional). GP26 must SIO, never PWM5A. Bench ranges/angles/settle/echo deadlines first; prove stop-before-scan and sensor/scan fault handling. No bypass until actual clearance envelope and line-recovery criteria are agreed and measured. |

All owners must measure worst-case RTOS execution, stack headroom and SMP stop
latency under network load; host fakes and static stack reports cannot establish
these. Startup preserves professor demonstration dependencies and all subsystem
initialization precedes starts. No hardware activation was performed.


### Results and reproducible commands

All rows refer to starting HEAD plus the uncommitted shared source hashes in
the evidence archive. The default/optimized runner checks exact unique IDs.

| Verification | Actual outcome |
|---|---|
| Shared default and O2/-fshort-enums | 54/54 each (INT-03..56); four adapter mutations detected each |
| Shared strict ARM | 16/16 objects, SMP=0/1 x APP_TEST=0/1; not intended-profile linking |
| Buddy 1 existing full host script | Codec 33, backoff 15, MQTT 33, Wi-Fi 17 checks pass; threaded application 40 PASS/3 FAIL (U14b/U14d/U16b). Serial 40 PASS/0 FAIL, with concurrency/held-lock checks skipped; not equivalent coverage. |
| Buddy 2 default and optimized | 138/138 each; eight mutations, timeout failure check and host-only guards pass; 19-file style subset passes |
| Buddy 3 existing production runner | Six functional suites compile/run successfully; overall runner FAIL solely from CRLF style check. Disposable LF-copy check passes; original failure preserved. |
| Buddy 4 existing current suite commands | 79 unique cases pass, strict C99/O2, GCC analyzer; includes runtime/transport fakes |
| Buddy 5 existing two host suites | 46 unique groups S5-U02..47 pass, strict C99/O2 and GCC analyzer; U01 physical/unverified |
| Full normal, TEST=1, NET=lwip MQTT=1, S4_IMU=1 | Each Make exits 2 on installed SDK layout, missing pico.h; no intended ELF/UF2 or memory result |
| Full CONSOLE=uart WIFI=none | Make/link/UF2 PASS; text/data/bss = 114740/1088/30852 bytes |
| Full CONSOLE=uart WIFI=none TEST=1 | Make/link/UF2 PASS; text/data/bss = 186832/1088/39696 bytes |

Host compiler: installed GCC 14.2; ARM: GCC 10.3.1. Shared and S2 strict warning
profiles include conversion/sign conversion, shadow, prototypes, format and
`-fanalyzer`. Existing S1 codec conversion warnings remain visible (only their
error promotion is relaxed in the unchanged shared runner). New shared adapter
has no strict warning/analyzer finding. Source diff whitespace check passes.
These checks are not full Barr-C certification: existing shared long collector/
owner functions and RTOS callback casts remain; no broad cosmetic refactor was
introduced. Added logic uses fixed-width values, bounded loops, copied snapshots
and no dynamic allocation. Static compiler stack frame for vehicle_task is 912
bytes against its 8192-byte allocation; nested call/interrupt/SMP headroom is not
measured. UART memory numbers exclude USB/Wi-Fi/IMU-enabled resources.

Commands (paths to installed compilers are recorded verbatim in evidence):

```text
python testing/integration/verify_integration.py --label software_reviewed_host
python testing/integration/verify_integration.py --arm --label software_reviewed_arm
python testing/subsystem2/verify_motion.py --strict --analyze --label software_integration
python testing/subsystem2/verify_motion.py --style
S1_NO_LOG=1 CC=/ucrt64/bin/gcc bash testing/subsystem1/host/run_host_tests.sh
S1_NO_LOG=1 S1_FORCE_NO_THREADS=1 CC=/ucrt64/bin/gcc bash testing/subsystem1/host/run_host_tests.sh
python testing/subsystem3/verify_production_timing.py --gcc C:/msys64/ucrt64/bin/gcc.exe --log testing/integration/logs/software_s3.json
cd build_make
make -j8
make TEST=1 -j8
make NET=lwip MQTT=1 -j8
make S4_IMU=1 -j8
make CONSOLE=uart WIFI=none -j8
make CONSOLE=uart WIFI=none TEST=1 -j8
```

S4/S5 exact compiler commands and individual cases are in `software_s4_s5.log`
inside the archive; they use existing sources/harnesses, no replacement tests.
Build environment: MSYS2 shell with `/ucrt64/bin`, `/usr/bin` and installed ARM
bin on PATH, `PICO_SDK_PATH=C:/PROGRA~1/RASPBE~2/PICOSD~1.1/pico-sdk`.
An installed compiler is separate from the SDK. Use an approved compatible SDK
at its own path with required submodules rather than installing silently.

### Independent review and hygiene verdict

The final diff changes only the shared runtime, existing integration harness/
runner, TEAM section 7, this record and one evidence archive. Existing buddy
C/H implementations and tests, GPIO dispatcher, mission algorithms, professor
Makefiles/kernel/configuration, root instructions and reference documents remain
unchanged. All five subsystem source trees were inspected; the presence of code
is not proof that live acquisition exists. No default profile/setup caller was
added, and missing S5 keeps movement blocked even with fresh line feedback.

Reviewed latch-before-ordinary-work sequencing, task start retry, S3 acquisition
vs read timestamps/pending IDs, raw/filtered agreement, independent S2 count
validity and S4 fault/event handling. No additional confirmed critical/high
shared defect remains in this focused change. Native interrupt installation,
scheduling/WCET and provider failures on real silicon remain unverified. Failed
provider tests are retained and assigned, not relabelled PASS.

One compressed evidence archive preserves every new run, including failed and
superseded attempts, with original-byte SHA-256 entries and final source hashes.
No earlier committed evidence is deleted. Disposable diagnostic images/maps,
current-run objects/dependencies and six S3 host executables are removed after
recording output hashes; temporary host/ARM scratch is already empty. Generated
outputs stay excluded from Git. Existing `.gitignore` covers build directories,
ELF/UF2/maps/objects/dependencies; a future shared exclusion for standalone `.su`,
`.exe` and Python cache files may be worthwhile, but no shared ignore change was
made. Do not remove legitimate historical evidence to reduce file count.

Recommendation: **PASS WITH CONDITIONS for review of this non-actuating shared
change; NOT a ready-to-flash integrated robot candidate.** First resolve SDK
build prerequisites and Buddy 1 threaded results, then Buddy 5 live APIs and the
telemetry contract. Sensor-only staged bring-up can be prepared; powered movement
requires further authorization, calibrated settings and closure of safety gates.


## Software completion verification (2026-10-16)

This section supersedes earlier current-status conclusions without deleting
historical results. Base HEAD: 5c08a5c76de39ffe18d9720b40d505a592508230,
branch codex/final-software-integration plus final uncommitted corrections.
These are source-hash-bound preliminary results, not a committed-revision claim.

### Corrections and self-review

S1 threaded failures came from incorrect fake interpretation of zero-valued
TWF_ANDW and an unfair busy reader exhausting its budget. Correct AND/OR
semantics have a direct regression; periodic test waits preserve all original
coherence, overlap and totals assertions. Host message capacity accommodates the
new copied frame. User-approved production telemetry v2 preserves signed int64
counts and uint64 sample time as decimal strings, independent validity and ages;
legacy JSON remains. Dashboard mapping/FR17 academic acceptance still needs
confirmation. Real frames are queued at most once per 100 ms.

S5 now owns explicitly configured core-1 ultrasonic/servo acquisition,
nonblocking scan deadlines, original capture timestamps, POL copied status and
an independent fault/cancel latch. Initialization cleanup releases owned IDs
and preserves original errors; failed starts may retry. Successful restart,
configuration updates and live fault reset are unsupported. The controller
uses actual forward readiness separately from full scan diagnostics.
APP_TEST=1 leaves the controller dormant to preserve professor self-test queue
ownership and simulator behavior.

Self-review checked widths/units/clock domains, partial cleanup, same-core ISR
exclusion versus cross-core mutex protection, bounded loops, PWM aliases and
startup interlocks. Late corrections enforce HC-SR04 quiet intervals throughout
scans and publish actual fault/cancel state/cause instead of stale MONITOR.
S2/S4 production, S3 production, app_main.c/app.h, GPIO dispatcher, professor
kernel/drivers/configuration and Makefiles remain unchanged. S3's style checker
alone normalizes CRLF checkout lines. No production synthetic ranges or default
mission/calibration setup were introduced.

Barr-C review: public-before-private bodies, fixed-width units, explicit
ownership, no allocation, bounded loops and end markers retained. RTOS callback
casts/register macros follow supplied ABI. Existing compact formatting/naming
is retained; this is not complete automated Barr-C certification or a cosmetic
rewrite of inherited source.

### Actual verification

| Scope | Final result |
|---|---|
| Integration default + optimized | 62 unique INT-03..INT-64 PASS/profile; INT-63 in APP_TEST=1 variant |
| Integration mutations | Five/profile detected: S1 latch, S3 getter, start rollback, IMU drain, count truncation |
| S1 host | 43 threaded, 40 codec, 15 backoff, 33 MQTT and 17 Wi-Fi checks PASS |
| S2 host | 138 unique groups/profile PASS; eight mutations and timeout/guard checks |
| S3 host | Six functional suites and style checker PASS |
| S4 host | 79 unique strict/analyzer cases PASS |
| S5 host | 71 unique S5-U02..U72 PASS/profile; real acquisition/task/MMIO with hardware fakes |
| ARM strict objects | 11 objects x SMP=0/1 x APP_TEST=0/1 = 44 PASS |
| Full firmware | Six USB/Wi-Fi profiles link; zero warning/error lines in final logs |

Commands:
python testing/integration/verify_integration.py --label completion_release_host
python testing/integration/verify_integration.py --arm --label completion_race_verified_arm
S1_NO_LOG=1 bash testing/subsystem1/host/run_host_tests.sh
S2: existing verify_motion.py runner (exact invocation/options in evidence).
S3: existing verify_production_timing.py runner.
S4: exact strict compiler/analyzer commands in completion_s4.log.

Evidence: logs/software_completion_evidence.zip contains final
completion_release_host_{default,optimized}.log,
completion_race_verified_arm_arm.log, completion_verified_s1.log,
completion_s3.json, completion_s4.log, S2 verify_completion_* and
completion_race_verified_sdk_*.log. Failed/superseded current-task runs remain
archived with byte hashes, not counted as final PASS. Existing
software_preparation_evidence.zip is preserved. The new manifest binds all
production/test/runner inputs, including new untracked sources, tool and SDK
identities, exact base revision and artifact hashes. Final host/ARM metadata
source hashes were compared against final files: no mismatches.
S1 raw MSYS Git metadata says unknown (Git unavailable in that shell);
the external manifest supplies the actual revision. Optional Windows
cross-build compiled; wine unavailable, so that executable was not run.

| New ID | Required behavior |
|---|---|
| INT-57 | S5 fault latch preempts configuration and ordinary provider reads |
| INT-58 | Busy obstacle status inhibits enable |
| INT-59 | Stale forward capture cannot establish readiness |
| INT-60 | Fresh forward-only status reaches READY without automatic enable |
| INT-61 | Real aggregator+codec preserve signed int64 extremes/source timestamp |
| INT-62 | 22 owner cycles emit three frames, respecting 100 ms cadence |
| INT-63 | APP_TEST=1 preserves five self-tests and dormant controller |
| INT-64 | Inconsistent age budgets reject; immutable valid setup copies IRQ hooks |

### Full firmware prerequisites and artifacts

Approved ignored local SDK build/deps/pico-sdk, tag 2.2.0:
a1438dff1d38bd9c65dbd693f0e5db4b9ae91779.
TinyUSB 86ad6e56c1700e85f1c5678607a762cfe3aa2f47;
CYW43 dd7568229f3bf7a37737b9e1ef250c26efe75b23;
lwIP 77dcd25a72509eb83f72b033d219b1d40cd8eb95.
ARM GNU 10.3.1, Make 4.4.1, host GCC 14.2.
No system-wide installation or professor build patch.
From build_make: export PICO_SDK_PATH=../build/deps/pico-sdk, then
make [options] "INCPATH=-I../include -I../config -I../kernel/knlinc -I../build/profiles" -j8.
Defaults SMP=1 CONSOLE=usb_cdc WIFI=cyw43 CRASH_REPORT=0.
Ignored build/profiles/wifi_secrets.h is a byte copy of published example
credentials: MQTT images prove linking, not configured network deliverables.

| Options | ELF/UF2 basename | text | data | bss bytes |
|---|---|---:|---:|---:|
| default | mtk3pico_smp1_usb_cdc_wifi | 341152 | 92 | 44484 |
| TEST=1 | mtk3pico_smp1_usb_cdc_wifi_test | 429808 | 1160 | 53332 |
| S4_IMU=1 | mtk3pico_smp1_usb_cdc_wifi_imu | 355316 | 1168 | 44496 |
| S4_IMU=1 TEST=1 | mtk3pico_smp1_usb_cdc_wifi_test_imu | 432480 | 1168 | 53340 |
| NET=lwip MQTT=1 S4_IMU=1 | mtk3pico_smp1_usb_cdc_wifi_lwip_mqtt_imu | 414724 | 1176 | 102152 |
| NET=lwip MQTT=1 S4_IMU=1 TEST=1 | mtk3pico_smp1_usb_cdc_wifi_lwip_mqtt_test_imu | 487848 | 1176 | 110996 |

Final ELF/UF2/maps remain in ignored build_make; artifact hashes are archived.
Static sections fit configured memory; this does not establish heap/stack
peaks, WCET, ISR latency or real SMP contention.

### Remaining gates and recommendation

Executable avoidance/moving line recovery remain incomplete: angular samples
alone cannot establish swept path, blind/rear clearance or acoustic errors.
Current planner returns CLEARANCE_UNVERIFIED and no actions. A validated
geometry/coverage/error model, geometric planner and controller execution are
required before claiming autonomous mission completion. No fabricated
clearance bypasses this gate.

No default vehicle_runtime_setup caller/calibration profile exists.
Explicit opt-in and mandatory fresh providers remain required. Before hardware:
confirm GP15 servo signal/supply/endpoints, HC-SR04 5 V operation/ECHO level
shifting, pulse/settling/range conversion, motor/encoder wiring/polarity/PID/
geometry, IR/barcodes, IMU mounting/traction, actual network credentials and
target fault/IRQ/timing/resource budgets. All physical/on-target tests remain
unverified. No firmware was flashed or motor activated.

PASS WITH CONDITIONS for review of this non-actuating software commit
candidate; full autonomous mission acceptance is incomplete. Controlled
sensor-only bring-up requires electrical/configuration review and separate
hardware authorization.

### File inventory and repository hygiene

Production and tests remain in established directories; existing guides were
updated rather than duplicated. One completion evidence archive retains all
new runs/hashes; prior unique history is preserved. Temporary host/ARM scratch,
objects/dependencies/stack reports and current-run S3 executables are removed;
final firmware deliverables, SDK and example fixture remain ignored.
No shared .gitignore edits. Future exclusions for stray .su/.exe/__pycache__
may be useful. No staging, commit, push, merge or branch change.

Exact changed-file inventory (new files marked new):

- TEAM.md
- app_program/subsystem1/s1_codec.c
- app_program/subsystem1/s1_codec.h
- app_program/subsystem1/subsystem1_test.c
- app_program/subsystem5/drv_s5.c (new)
- app_program/subsystem5/drv_s5.h (new)
- app_program/subsystem5/s5_acquire.c (new)
- app_program/subsystem5/s5_acquire.h (new)
- app_program/subsystem5/s5_live.c (new)
- app_program/subsystem5/s5_live.h (new)
- app_program/subsystem5/s5_obstacle_logic.h
- app_program/subsystem5/subsystem5.c
- app_program/vehicle.c
- app_program/vehicle.h
- app_program/vehicle_rt.c
- app_program/vehicle_rt.h
- docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md
- docs/buddy5/OBSTACLE_SYSTEM.md
- testing/integration/TEST_LOG.md
- testing/integration/logs/software_completion_evidence.zip (new)
- testing/integration/logs/software_preparation_evidence.zip (new)
- testing/integration/test_ports.h
- testing/integration/test_vehicle.c
- testing/integration/verify_integration.py
- testing/subsystem1/TEST_LOG.md
- testing/subsystem1/host/kernel_emulation.c
- testing/subsystem1/host/target_main.c
- testing/subsystem1/host/test_codec_host.c
- testing/subsystem3/TEST_LOG.md
- testing/subsystem3/check_barr_style.py
- testing/subsystem5/TEST_LOG.md
- testing/subsystem5/host/test_obstacle_runtime.c
- testing/subsystem5/host/test_s5_port.c (new)
- testing/subsystem5/host/test_s5_port.h (new)

Late self-review correction: an ISR capture can complete between the task's
clock read and echo copy. s5_acquire now defers consumption until the next
cycle while preserving the original echo deadline and timestamp. Extended U50
reproduces the interleaving; reintroduced defect is detected by mutation.
Final integration evidence uses completion_release_host; ARM/full-build evidence
uses completion_race_verified_*;
S1/S2/S3/S4 sources are unchanged since their recorded runs.

Final rerun: completion_release_host passes both profiles, all 62 integration
IDs and 71 S5 IDs each, five shared mutations/profile. The separate echo-race
mutation is detected. completion_race_verified_arm passes all 44 objects; all
six completion_race_verified_sdk profiles link without warnings. One earlier
optimized mutation compile hit the bounded 120 s tool timeout; that failed
attempt is archived explicitly and excluded from final PASS results.


## Configured bypass and recovery verification (2026-10-16)

This continuation supersedes the previous statement that configurable bypass
and moving recovery logic were missing. The code now implements both.
Actual robot authorization remains blocked by absent physical calibration and
verified environment/line-identity evidence; do not label that hardware-ready.

Base HEAD 5c08a5c76de39ffe18d9720b40d505a592508230, unchanged feature branch
codex/final-software-integration plus uncommitted changes. No Git staging,
commit, push, merge or physical operation. Final compiler input hashes match
the actual files; previous incomplete/mixed-review attempts are preserved but
excluded from final results.

### Implemented behavior and independent review

Existing S5 profile logic feeds a configured eight-action mirrored route:
outward turn/travel, forward turn/travel, inward turn/return, forward turn,
bounded forward LINE_FOUND search. Only actual existing S2 motion APIs actuate;
the coordinator observes admission, terminal ID, ACK and READY/standstill.
It never refreshes finite commands. Search success stops, acknowledges and
rearms S5 before explicitly resuming S3 tracking. If already centred after the
last turn, no unnecessary search movement is issued.

Every swept segment/rotation fits a verified free rectangle expanded for a
conservative body radius, tracking error and stopping. Evidence is scan-specific,
post-scan and valid for the whole route deadline; it must also establish the
original line and initial alignment. No current source manufactures certificates.
The optional POL copied certificate API is stopped-state intake only; owner
validation decides admission. Missing inputs keep movement inhibited.
Configured waits/actions/routes are bounded. PAUSE, emergency, unexpected mode,
new conflicting barcode, wrong/rejected terminal, sensor faults/staleness,
epoch change, expiry or failed search stop/fault rather than ambiguously resume.

Review corrected: legacy profile positive-right vs planner positive-left;
S3 PAUSED-on-LINE_FOUND handling; post-turn echo completion from an old trigger
(now uses pre-trigger source time); and clearing old scan-complete on recovery.
Shared setup checks footprint vs wheel track, motion speed and S3 recovery
timing compatibility. One blocked/incomplete sensor status is never fabricated
as a clear path.

Host-only MinGW C task fakes explicitly select the installed header's non-SEH
setjmp mode; instrumented normal execution failed at the exception-unwind
boundary after fake sleep, while debugger execution passed. The C-only
nonlocal-exit mode passes both optimization profiles; ARM code is unaffected.
No production assertion was removed. Earlier timeouts/failures remain evidence.
This is not a claim of measured target SMP timing or full Barr-C certification.

### Final regressions

Existing INT-03..INT-64 preserved; INT-63 still runs APP_TEST=1 startup.
INT-03..INT-81 = 79 unique IDs/profile PASS.
S5-U02..U88 = 87 unique IDs/profile PASS.
Each route command is also admitted by the real S2 move_plan logic using
explicit host geometry; fake statuses exercise actual coordinator handshakes.

| New ID | Definition / required result |
|---|---|
| INT-65 | Entire left eight-action route, repeated terminal ACK, stop/search/rearm/resume |
| INT-66 | Entire mirrored right route |
| INT-67 | Unverified free-space evidence cannot issue motion |
| INT-68 | Observe S3 mode receipt before command; finite command not refreshed |
| INT-69 | Action timeout faults |
| INT-70 | PAUSE during route faults safely |
| INT-71 | Certificate expiry faults |
| INT-72 | Search distance DONE without LINE_FOUND fails |
| INT-73 | Old found event ignored; fresh S3 PAUSED-on-found stops correctly |
| INT-74 | Wrong terminal ID faults |
| INT-75 | Invalid line sensing faults active route |
| INT-76 | Changed scan epoch faults |
| INT-77 | Fresh echo from pre-turn trigger cannot authorize translation |
| INT-78 | Unexpected active S3 mode faults |
| INT-79 | Invalid route speed/configuration rejects |
| INT-80 | Contradictory obstacle/free-space claim inhibits motion |
| INT-81 | Missing original-line identity inhibits motion |

Existing five mutation checks preserved; three new mutations remove clearance
authorization, weaken required search success and remove new-heading trigger
checking. All eight detected in both profiles. Source-bound final commands:
python testing/integration/verify_integration.py --label route_verified_host
python testing/integration/verify_integration.py --arm --label route_verified_arm
The latter passes 44 strict ARM objects (11 x SMP0/1 x TEST0/1).

Other reruns: S1 original host script (43 threaded / 41 codec / network suites),
S2 original strict/analyzer runner plus separate style subset, S3 original
six-suite/style timing runner, S4 original 79 strict/analyzer commands replayed
in temporary space. Raw results and exact commands are archived under
route_final_s1.log, verify_route_final_*, route_final_s3.json and route_final_s4.log.
No on-target or physical result is asserted.

### Firmware profile inclusion versus activation

All six use unchanged Make rules, SMP=1, CONSOLE=usb_cdc, WIFI=cyw43,
CRASH_REPORT=0 and approved local SDK 2.2.0. All include S5 acquisition and the
complete configured controller code. All still lack a default mission/calibration
caller and clearance source, so there is no automatic motor/servo activation.

| Profile options | Wi-Fi / IP / MQTT | IMU runtime | S5 owner | Vehicle task |
|---|---|---|---|---|
| default | driver / no lwIP / no MQTT | disabled | starts unconfigured | runs interlocked |
| TEST=1 | driver / no lwIP / no MQTT | disabled | starts unconfigured | dormant for self-tests |
| S4_IMU=1 | driver / no lwIP / no MQTT | enabled by build | starts unconfigured | runs interlocked |
| S4_IMU=1 TEST=1 | driver / no lwIP / no MQTT | enabled by build | starts unconfigured | dormant for self-tests |
| NET=lwip MQTT=1 S4_IMU=1 | driver / lwIP / MQTT | enabled by build | starts unconfigured | runs interlocked |
| NET=lwip MQTT=1 S4_IMU=1 TEST=1 | driver / lwIP / MQTT | enabled by build | starts unconfigured | dormant for self-tests |

Source inclusion does not prove valid sensor data or connectivity. IMU-enabled
images still require mounting/I2C/calibration approval before hardware use.
Network builds use the ignored published-example credential fixture, not a
configured live network. Exact commands, code sizes and artifact hashes are
recorded in route_verified_sdk_* logs and the archive manifest.

### Remaining evidence, hygiene and readiness

No maintained dashboard decoder exists in the repository. Consumer v2 mapping,
BigInt handling, null/validity semantics and 239-byte bound are documented in
docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md. FR17 semantic fields are retained;
academic acceptance of compact names remains an external agreement.

Required physical inputs/evidence: body/axle/sensor geometry, obstacle width/
depth bound, speed/PID/turn calibration, cumulative tracking and stopping bounds,
servo/ultrasonic calibration/electrical safety, static-scene free corridor
coverage including blind/rear sectors and original-line uniqueness/alignment.
A single HC-SR04 scan cannot independently establish all those corridors.
Autonomous evidence production requires a validated environment/coverage model
or additional sensing; calibration numbers alone cannot supply missing
observability. There is no fabricated production fallback.

Generic unbounded line search, reversing into unobserved space and arbitrary
branch reacquisition are deliberately unsupported; bounded nominal forward
recovery is implemented and faults on failure.

Independent review: all prior 34 paths retained for production, regression,
documentation or historical evidence; four additional existing files changed
for S5 planning/runtime and its tests. No new hierarchy, runner, guide or
test implementation duplicates were introduced. The old range-only planner
retains its public guard contract; the configured planner adds the missing
evidence-bearing route, not a second competing motion owner. Temporary outputs
are removed after verification; final firmware images/SDK remain ignored.
Historical raw evidence is preserved in the existing archives with manifests.
No professor kernel, Makefile, shared config, app_main.c/app.h or S2/S3/S4
production code changed. Barr-C exceptions/retained compact inherited style
remain manual-review matters, not a false automated compliance certificate.

Recommendation: PASS WITH CONDITIONS for final reviewed software integration
commit. Planning/execution/recovery logic is software-verified; autonomous
hardware acceptance remains gated by the explicitly listed evidence.

Final results: 79 integration and 87 S5 unique IDs per host profile; eight
mutations/profile detected; 138 S2 groups/profile plus its eight mutations and
timeout guards; S1 43 threaded / 41 codec / 15 backoff / 33 MQTT / 17 Wi-Fi;
S3 six suites plus style; S4 79 cases; 44 ARM objects; six full firmware links
PASS with zero warning/error lines in the final build logs. Final recorded
compiler inputs were compared with the frozen actual files: no differences.
All results are host/build evidence; no hardware operation or safety validation.

| Final profile | text | data | bss bytes |
|---|---:|---:|---:|
| normal | 345364 | 92 | 45092 |
| test | 434024 | 1160 | 53940 |
| imu | 359528 | 1168 | 45104 |
| imu_test | 436688 | 1168 | 53948 |
| mqtt | 418936 | 1176 | 102760 |
| mqtt_test | 492064 | 1176 | 111604 |

Exact final uncommitted inventory: 28 modified tracked paths + 10 new paths.
- TEAM.md
- app_program/subsystem1/s1_codec.c
- app_program/subsystem1/s1_codec.h
- app_program/subsystem1/subsystem1_test.c
- app_program/subsystem5/drv_s5.c (new)
- app_program/subsystem5/drv_s5.h (new)
- app_program/subsystem5/s5_acquire.c (new)
- app_program/subsystem5/s5_acquire.h (new)
- app_program/subsystem5/s5_live.c (new)
- app_program/subsystem5/s5_live.h (new)
- app_program/subsystem5/s5_obstacle_logic.c
- app_program/subsystem5/s5_obstacle_logic.h
- app_program/subsystem5/s5_obstacle_runtime.c
- app_program/subsystem5/s5_obstacle_runtime.h
- app_program/subsystem5/subsystem5.c
- app_program/vehicle.c
- app_program/vehicle.h
- app_program/vehicle_rt.c
- app_program/vehicle_rt.h
- docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md
- docs/buddy5/OBSTACLE_SYSTEM.md
- testing/integration/TEST_LOG.md
- testing/integration/logs/software_completion_evidence.zip (new)
- testing/integration/logs/software_preparation_evidence.zip (new)
- testing/integration/test_ports.h
- testing/integration/test_vehicle.c
- testing/integration/verify_integration.py
- testing/subsystem1/TEST_LOG.md
- testing/subsystem1/host/kernel_emulation.c
- testing/subsystem1/host/target_main.c
- testing/subsystem1/host/test_codec_host.c
- testing/subsystem3/TEST_LOG.md
- testing/subsystem3/check_barr_style.py
- testing/subsystem5/TEST_LOG.md
- testing/subsystem5/host/test_obstacle_logic.c
- testing/subsystem5/host/test_obstacle_runtime.c
- testing/subsystem5/host/test_s5_port.c (new)
- testing/subsystem5/host/test_s5_port.h (new)

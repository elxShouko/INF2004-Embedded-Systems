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

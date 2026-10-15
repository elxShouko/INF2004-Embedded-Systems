# Subsystem 3 — test log

**Subsystem:** Consumer (example)  ·  **Student ID:** <student ID>  ·  **Code:** `app_program/subsystem3/`

See `testing/README.md` for how to run tests and record results.

## Test cases

Define a test here before running it. Test IDs never change.

| ID | What is tested | Requirement / interface | Method | Steps / how | Expected result |
|---|---|---|---|---|---|
| S3-U01 | statistics self-consistent (min ≤ max, 0..100) | | auto | `subsystem3_test.c` | `[TEST] S3-U01 PASS` |
| S3-U02 | Barr-C mechanical review subset | Barr-C:2018 | host | Run `check_barr_style.py` | No checked-rule violations; exceptions remain explicit |
| S3-U03 | IR, barcode/navigation and junction regressions | Member 3 algorithms | host | Compile/run the three existing host suites in `logs/` with integrated driver sources | All assertions pass |
| S3-U04 | Owner-task state machine regression | S3 queue/state contract | host simulation | Compile/run `logs/member3_rtos_test.c` against integrated driver sources | All assertions pass |
| S3-U05 | Public data ABI preservation | Existing S3 interfaces | host | Compare sizes, offsets and enum values against the pre-edit snapshot | Identical reports |
| S3-U06 | Default integrated build | Professor Make foundation | compile only | `cd build_make; make -j8` | Default SMP/USB/Wi-Fi-driver ELF and UF2 produced |
| S3-U07 | Integrated test-profile build | Template test discovery | compile only | `cd build_make; make TEST=1 -j8` | Test ELF and UF2 produced; no claim of execution |
| S3-U08 | State-transition differential trace | Behaviour preservation | host simulation | Compare 40,000 deterministic sample transitions before/after cleanup | Identical trace hash |
| S3-U09 | Standalone diagnostic relocation | Optional CMake diagnostic target | compile only | Compare moved source with HEAD, then build `member3_ir_diagnostic` | Source identical; target produces UF2 from `diagnostics/` |


| S3-U10 | Production tick configuration | 10 ms kernel tick | host simulation | Run production suite, accepted 10..100 ms multiples and rejected periods | Only supported periods accepted; undersized gap budget rejected |
| S3-U11 | Sampled barcode waveform | Production polling | host simulation | Independent *A* edges, both directions, phase sweep, 10/20 ms and jitter | Exactly one valid event per resolvable fixture |
| S3-U12 | Sampling jitter and gaps | Sample freshness | host simulation | Boundary gap, excessive gap, reversed clock | Allowed gap accepted; invalid timing latches fault |
| S3-U13 | Narrow-element aliasing | Polling limitation | host simulation | Sample subperiod frame and hidden pulse | No valid frame; missed pulse remains unobservable |
| S3-U14 | Sustained input load | Owner task fairness | host simulation | Run real task loop for 50 fake cycles with persistent input | At most four messages per sample; sensing continues |
| S3-U15 | Output overload | Latched fault/status | host simulation | Undrained bounded fake output under reply flood | Fault/paused status; sensor reads continue |
| S3-U16 | Manoeuvre and recovery handoffs | Provider contract | host simulation | Wrong/matched/duplicate completion, found, resume, unchanged observation | Correlated completion; explicit resume; fresh status without repeat event |
| S3-U17 | Bypass and sensing failures | Provider fault handling | host simulation | Bypass, recovery timeout, malformed sample | Suppressed bypass events; faults latch and pause |
| S3-U18 | Stable reacquisition interruption | Recovery evidence | host simulation | Interrupt centred run with raw white before filter settles | No LINE_FOUND until a new uninterrupted centred interval |
| S3-U19 | Actual barcode operating envelope | NFR4 / physical sensing | manual, pending | Follow docs/buddy3/production-timing-and-handoff.md | Measured reliable speed range and both-direction recognition evidence |

*Method:* **auto** = `TEST_CHECK` in code, result printed on the console;
**manual** = a person observes the result (LED, scope, meter, phone app…).

## Results

Add a new row for every run. Never edit or delete old rows.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| YYYY-MM-DD | S3-U01 | `abc1234` | | PASS / FAIL | `logs/YYYY-MM-DD_….txt` | |

### 2026-10-15 Barr-C cleanup verification

Tested `buddy3-barcode` at HEAD `a508994331acf0adaa9acff1927cdaeb9f232abe`
plus uncommitted Member 3 changes. Exact C/header/format-config fingerprint:
`de1f2c9e45f63c67bacdbc7c4ae76e039b8803fb9660a89d6192cb9030e038a3`.
No commit was created; the fingerprint identifies the tested working tree.

| Date | Test ID | Commit / working tree | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S3-U02 | `a508994` + fingerprint above | Codex | PASS, checked subset | `logs/2026-10-15_barr-host-results.json` | 26 C/header files; longest function 88 physical lines; not a complete conformance certification |
| 2026-10-15 | S3-U03 | Same working tree | Codex | PASS, host | `logs/2026-10-15_barr-host-results.json` | All three algorithm suites passed before and after edits |
| 2026-10-15 | S3-U04 | Same working tree | Codex | PASS, host simulation | `logs/2026-10-15_barr-host-results.json` | State machine uses fake kernel calls; no SMP/on-target claim |
| 2026-10-15 | S3-U05 | Same working tree | Codex | PASS, host ABI comparison | `logs/2026-10-15_barr-host-results.json` | Checked public sizes, offsets and enum values match the baseline |
| 2026-10-15 | S3-U06 | Same working tree | Codex | PASS, compilation only | `logs/2026-10-15_barr-normal-build.txt` | `make -j8`; default SMP/USB/CYW43 profile; UF2 generated, not flashed |
| 2026-10-15 | S3-U07 | Same working tree | Codex | PASS, compilation only | `logs/2026-10-15_barr-test-build.txt` | `make TEST=1 -j8`; test UF2 generated; tests not executed on board |
| 2026-10-15 | S3-U08 | Same working tree | Codex | PASS, differential simulation | `logs/2026-10-15_barr-host-results.json` | 40,000 input samples; both trace hashes `749edf30d9302f8f` |

Commands, installed-toolchain environment, build profiles and firmware hashes
are in `logs/2026-10-15_barr-run.json`. All Subsystem 3 sources also passed
ARM C99 syntax checking with `-Wall -Wextra -Werror`; the exact command and
output are in `logs/2026-10-15_barr-arm-c99.json`.

The host rerun command from the repository root was:

```powershell
& 'C:\Users\linco\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' testing/subsystem3/verify_barr_refactor.py --gcc 'C:\msys64\ucrt64\bin\gcc.exe' --log testing/subsystem3/logs/2026-10-15_barr-host-results.json
```

Compatibility exceptions S3-DEV-01 through S3-DEV-03 remain pending approval
in `app_program/subsystem3/BARR_COMPLIANCE.md`. Physical calibration,
hardware behavior, real queue overload handling and scheduling remain untested.

### 2026-10-15 compatibility-exception approval

After the verification run, the requesting user approved S3-DEV-01 through
S3-DEV-03, answering: "Approve the documented compatibility exceptions."
The approved scope and rationale are recorded in
`app_program/subsystem3/BARR_COMPLIANCE.md`. This supersedes the pending status
recorded above; it does not change the historical test results or authorize
unrelated shared changes. Only this approval record and the compliance report
were updated after approval; the tested C/header/format-config fingerprint
remains unchanged. No additional build or hardware test is claimed.

### 2026-10-15 standalone diagnostic relocation

| Date | Test ID | Commit / working tree | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S3-U09 | `a405c5aa2752e39bdcd3d4e0b415d2126158ea15` + uncommitted relocation, CMake path and documentation changes | Codex | PASS, compilation only | `logs/2026-10-15_diagnostic-relocation-build.txt` | Source bytes preserved; optional diagnostic rebuilt from `diagnostics/`; UF2 produced; not flashed |

Moved `examples/member3_ir_diagnostic.c` to
`testing/subsystem3/diagnostics/member3_ir_diagnostic.c`. Both source SHA256
values were `cc3f6a8055bcbaf086e0d9d5733b867951c39f04e3dd73c50e32b8b7e7b94192`.
The integrated Make application and other subsystem implementations were not
changed. Used the existing Ninja build directory, Pico SDK 2.3.1, Arm GNU
15.2.Rel1 and installed picotool, with dependency fetching disabled.

Exact commands from the repository root (PowerShell; both exited zero):

```powershell
$env:PICO_SDK_PATH = "$env:USERPROFILE/.pico-sdk/sdk/2.3.1"
$env:PICO_TOOLCHAIN_PATH = "$env:USERPROFILE/.pico-sdk/toolchain/15_2_Rel1"
& "$env:USERPROFILE/.pico-sdk/cmake/v4.3.4/bin/cmake.exe" -S . -B build/member3-pico -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DPICO_SDK_FETCH_FROM_GIT=OFF -DBUILD_MEMBER3_IR_DIAGNOSTIC=ON
& "$env:USERPROFILE/.pico-sdk/cmake/v4.3.4/bin/cmake.exe" --build build/member3-pico --target member3_ir_diagnostic -j 4
```

Output: `build/member3-pico/member3_ir_diagnostic.uf2`. No hardware test was run.

### 2026-10-15 documentation organisation

Buddy 3's main guide, integration guide and Barr-C review are now grouped under
`docs/buddy3/`, indexed by `docs/buddy3/README.md`. Earlier paths in this log and
archived evidence are preserved as historical records. In particular, the
Barr-C report moved from `app_program/subsystem3/BARR_COMPLIANCE.md` to
`docs/buddy3/BARR_COMPLIANCE.md`. Source changes for this organisation update
only documentation comments; no executable statements or interfaces changed.
No new firmware build or hardware result is claimed for the documentation move.

Static checks passed on HEAD `a405c5aa2752e39bdcd3d4e0b415d2126158ea15` plus
the uncommitted diagnostic/documentation relocations: all 10 local Markdown
link destinations in the affected guides/indexes exist; stripping comments from
the 20 modified tracked C/header files matches HEAD; and
`python testing/subsystem3/check_barr_style.py` passed its declared subset for
26 files (longest function 88 lines). The whitespace check
`git -c core.whitespace=-blank-at-eof diff --check` also passed.

### 2026-10-15 production timing and recovery verification

Tested branch `buddy3-barcode`, HEAD
`4240895a455679ad0a3996057e03723cf2e9daea` plus the uncommitted S3 code,
header comments, host tests and Buddy 3 documentation changes in this run.
No commit was authorised or created. Exact tested source/test hashes and
compile/run commands are in `logs/2026-10-15_production-host-results.json`.
Build environment, commands, profiles and UF2 hashes are in
`logs/2026-10-15_production-run.json`.

| Date | Test ID | Commit / working tree | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S3-U18 | `4240895` + new production test, before recovery fix | Codex | FAIL, host simulation | `logs/2026-10-15_production-before-fix.txt` | At the confirmation deadline, current raw white still emitted LINE_FOUND because the filter retained centred |
| 2026-10-15 | S3-U02 | `4240895` + manifest above | Codex | PASS, checked subset | `logs/2026-10-15_production-host-results.json` | 26 application C/header files; longest function 88 lines; not full Barr certification |
| 2026-10-15 | S3-U03, S3-U04 | Same working tree | Codex | PASS, host simulation | `logs/2026-10-15_production-host-results.json` | Existing IR, navigation, junction and retained 1 ms logical owner suite |
| 2026-10-15 | S3-U10 through S3-U17 | Same working tree | Codex | PASS, host simulation | `logs/2026-10-15_production-host-results.json` | 10 ms production fixture, accepted multiples, 10/20 ms sampled frames, jitter/gaps, aliasing, task-loop queue load and provider handoffs |
| 2026-10-15 | S3-U18 | Same working tree, after recovery fix | Codex | PASS, host simulation | `logs/2026-10-15_production-host-results.json` | Raw interruption restarts confirmation; stable sampled centred run is then required |
| 2026-10-15 | S3-U06 | Same application working tree | Codex | PASS, compilation only | `logs/2026-10-15_production-normal-build.txt` | `cd build_make; make -j8`; normal UF2 generated, not flashed |
| 2026-10-15 | S3-U07 | Same application working tree | Codex | PASS, compilation only | `logs/2026-10-15_production-test-build.txt` | `cd build_make; make TEST=1 -j8`; test UF2 generated, not executed on board |
| 2026-10-15 | S3-U19 | Not tested | � | PENDING, hardware | `docs/buddy3/production-timing-and-handoff.md` | Minimum elements, optics, polarity, actual speed, quiet/timeout settings and recognition trials still unmeasured |

Build profiles: `SMP=1 CONSOLE=usb_cdc WIFI=cyw43 NET=none MQTT=0
CRASH_REPORT=0 S4_IMU=0`, with `TEST=0` and `TEST=1` respectively.
Used already-installed MSYS2, Arm GNU 15.2.Rel1 and Pico SDK 2.3.1. Initial
sandboxed MSYS2 startup failed before Make with Windows object access error
`0xC0000022`; the authorised build retry outside the sandbox passed. No tools
or dependencies were installed or fetched.

Run the host checks from the repository root:

```powershell
& 'C:/Users/linco/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' testing/subsystem3/verify_production_timing.py --gcc C:/msys64/ucrt64/bin/gcc.exe --log testing/subsystem3/logs/2026-10-15_production-host-results.json
```

The new owner-loop load fake is not an RTOS scheduler or SMP simulator. The
60 ms narrow/180 ms wide *A* waveform is synthetic, not a calibrated speed.
Subperiod elements can disappear without any timestamp-gap fault. Invalid
payload coverage is not proof of disconnected/stuck-sensor detection.
Physical calibration and controller-connected stop, refresh and original-route
recovery tests remain pending; integration IDs require agreement with owners.
The controller handoff guide explicitly distinguishes existing provider
behaviour from proposed policy, clock conversion and unvalidated parameters.
Shared interfaces/configuration, other subsystem implementations, motor
outputs and retained demonstration dependencies were not changed.

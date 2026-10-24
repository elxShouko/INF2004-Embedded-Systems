# Buddy 5 pre-commit review — 2026-10-21

Base: `d4c91f271c9801802e3994967c96e9ec8fcba38c` plus the current uncommitted
Buddy 5 changes, on `buddy5-obstacle-actual`. No staging, commit, push, remote
fetch, branch mutation, flashing or physical operation was performed.

## Verdict

All available host suites and the checked ARM objects pass. No Buddy 1–4
production code, controller code, shared interfaces, Makefiles, configuration,
kernel, platform or library files were changed. This supports a scoped Buddy 5
software change; it does not establish full firmware or hardware readiness.

Full normal/TEST firmware linking is still blocked by the installed SDK setup.
The prior Make attempts with SDK 1.5.1 failed in unchanged USB support on missing
`pico.h`; the README specifies SDK 2.2.0. A request to download SDK 2.2.0 into
ignored `build/deps` is pending user authorization, as required by AGENTS.md.
No dependency was downloaded without that authorization.

## Results on the current working tree

| Area | Result | Evidence |
|---|---|---|
| Buddy 1 codec/backoff/MQTT/Wi-Fi | PASS: 41 + 15 + 33 + 17 checks | `testing/subsystem5/logs/precommit_20261021_peers.json` |
| Buddy 1 application/concurrent status | PASS: 43 checks, actual host pthreads enabled | Same peer log; host emulation, not target SMP |
| Buddy 2 | PASS: 95 pure + 43 adapter/lifecycle/feedback groups in each default/optimized profile; eight focused mutations and timeout/host guards detected | `testing/subsystem2/logs/verify_buddy5_precommit_20261021_*` |
| Buddy 2 ARM | PASS: 19 objects for each SMP=0/1, TEST=0/1 combination (76 compilations) | `testing/subsystem2/logs/verify_buddy5_precommit_arm_20261021_*` |
| Buddy 3 | PASS: style subset and six IR/navigation/junction/RTOS/production/feedback host suites | `testing/subsystem5/logs/precommit_20261021_s3.json` |
| Buddy 4 | PASS: 54 core + 23 runtime + 2 transport cases (79 total) | Peer log above |
| Buddy 5 | PASS: all 113 existing/new S5-U02..U114 cases per default/optimized profile | `testing/integration/logs/precommit_20261021_host_*` plus `testing/subsystem5/logs/precommit_20261021_representation.txt` |
| Controller/shared integration | PASS: 79 integration IDs and eight deliberate regression mutations detected per profile | `testing/integration/logs/precommit_20261021_host_*` |
| Integration/provider ARM | PASS: 11 objects for each SMP=0/1, TEST=0/1 combination (44 compilations) | `testing/integration/logs/precommit_20261021_arm_*` |
| Numerical negative controls | PASS: both deliberately reintroduced acquisition bugs detected per profile | Representation log above |
| Change hygiene | PASS: only four Buddy 5 production files changed; whitespace check clean; no reference PDFs, credentials or generated binaries in proposed files; credential pattern scan found no matches | `testing/subsystem5/logs/precommit_20261021_manifest.json` |
| Full normal/TEST firmware | BLOCKED: installed SDK mismatch; no linked image claimed | `testing/subsystem5/logs/buddy5_20261021_make_sdk151.txt` |
| On-target and physical tests | NOT RUN | S5-U01, S5-U115..U119 and broader mission acceptance remain outstanding |

An expected timeout diagnostic in the Buddy 2 negative-control run is a tested
guard, not a failed suite. Intentional FAIL lines in mutation logs likewise
prove that the test detects the deliberately broken scratch implementation.

The peer runner reproduces the source lists, flags and cases from S1/S4's
existing shell entrypoints using Python subprocess arguments, avoiding Windows
shell/path issues. It edits no peer files and records the exact compiler
commands and source hashes. It uses host fakes; it does not connect to Wi-Fi,
MQTT or hardware. No tests were weakened or skipped to manufacture a PASS.

## Review scope

The production diff consists of:

- `drv_s5.h/.c`: four ISR flags become one explicit byte state; ISR-shared
  timing fields are volatile, with same-core IRQ exclusion/compiler barriers
  for coherent task snapshots; terminal error/completion captures are retained.
- `s5_acquire.h/.c`: private counts/state become bytes, private owner members
  are reordered, invalid counts/states fail closed, raw pulse limits are checked
  before cm rounding, and impossible trigger-to-capture timing is rejected.

Shared public scan/profile/plan/policy/live layouts and function signatures
remain unchanged. Buddy 2 retains exclusive motor actuation. A complete rebuild
is required because the internal Buddy 5 owner layouts changed.

Supporting changes are the representation tests, MMIO regressions, measurement
and verification runners, Buddy 5 documentation, append-only test records and
saved evidence. Existing failed build/probe evidence is deliberately preserved
and labelled; it is not presented as successful verification.

Review the new source/tests/docs and selected evidence paths explicitly when
staging. Generated objects/executables/firmware remain under ignored build
directories. Do not include `reference_docs/`, credentials or scratch artifacts.
Local HEAD is equal to local main and the locally recorded upstream; no fresh
remote comparison was made. Actual commit/push still requires explicit approval.

## Reproduction

From the repository root, using installed tools:

```powershell
$pythonExe = 'C:/Users/User/AppData/Local/Programs/Python/Python314/python.exe'
& $pythonExe testing/subsystem5/verify_peer_regressions.py --label NEW_peers
& $pythonExe testing/subsystem2/verify_motion.py --strict --analyze --label NEW_s2
& $pythonExe testing/subsystem2/verify_motion.py --strict --arm --label NEW_s2_arm
& $pythonExe testing/subsystem3/verify_production_timing.py --gcc C:/msys64/ucrt64/bin/gcc.exe --log testing/subsystem5/logs/NEW_s3.json
& $pythonExe testing/subsystem5/verify_representation.py --label NEW_s5
& $pythonExe testing/integration/verify_integration.py --label NEW_integration
& $pythonExe testing/integration/verify_integration.py --label NEW_arm --arm
git diff --check
```

Use unused evidence labels. Hardware calibration, encoder/servo/ultrasonic
electrical verification, real stop/IRQ/stack timing, verified free-space and
original-line identity evidence, physical bypass/recovery and real network
operation remain separate acceptance gates.

# Buddy 2 PR preflight — preliminary working-tree evidence

Recorded UTC: 2026-10-14T14:13:40.750913+00:00

- Branch: `buddy2-motion-control-pr`; fetched origin/main: `2886fa8a0118185674f59964ab832d7c4e3ceabb`.
- Original development checkout is retained unchanged.
- No implementation commit, evidence commit or push has occurred. Git staging
  and committing were blocked by automatic approval review pending direct
  chat authorisation. This manifest does not establish a tested commit.
- All 38 C/H files and the verification script/recipe match the original
  development checkout byte-for-byte. Production C/H was not refactored.
- Three technical narratives were relocated into docs/buddy2 in this isolated
  checkout; source README added, live links updated and definition table repaired.
- All 97 definitions and historical results preserved. All 23 original log-folder
  files, including manifest and .gitkeep, preserved byte-for-byte.
- 1,457 live Markdown links checked, zero broken. Flat Make discovery and
  TEST=1 test registration inspected. No Make invocation or hardware execution.

## Fresh host checks

Commands, from isolated repository root:

```sh
python -B app_program/subsystem2/verify_motion.py --style
python -B app_program/subsystem2/verify_motion.py --label precommit20261014a --analyze
```

Style subset: 17 files PASS. Strict GCC C99 default and O2/fshort-enums
profiles with -fanalyzer: 95 unique groups PASS in each, S2-U03 through S2-U97,
compiler and process exits zero. S2-U01/U02 excluded from host runner; no target
execution evidence. Raw commands and outputs retained below. Generated host
executables removed; no generated executable is intended for Git.

These are preliminary uncommitted checks. The requested formal committed-revision
rerun and follow-up evidence commit must occur after Git authorisation. Full
firmware linking remains unverified due to the documented SDK mismatch.

| Raw evidence | SHA256 |
|---|---|
| [2026-10-14_precommit20261014a_default.txt](2026-10-14_precommit20261014a_default.txt) | `76b3c98d2e3b908b2b139bc797086092149511efd5d6423f2d1913f8a90908d3` |
| [2026-10-14_precommit20261014a_optimized.txt](2026-10-14_precommit20261014a_optimized.txt) | `b934e53f1a84ebdfb7a9aef09f601e5cbb8d6a6353fd04a3afc3667861e736f5` |

## Exact proposed PR changed-file inventory

69 paths, including this manifest. Existing unchanged task_producer.c and
logs/.gitkeep remain inherited from main and are not changes. No root AGENTS.md,
reference_docs/, build_make logs, teammate source or shared configuration is
transferred. This list must be reviewed again after formal evidence is added.

- `app_program/subsystem2/README.md`
- `app_program/subsystem2/drv_encoder.c`
- `app_program/subsystem2/drv_encoder.h`
- `app_program/subsystem2/drv_motor.c`
- `app_program/subsystem2/drv_motor.h`
- `app_program/subsystem2/drv_rp2040.c`
- `app_program/subsystem2/drv_rp2040.h`
- `app_program/subsystem2/enc_capture.c`
- `app_program/subsystem2/enc_capture.h`
- `app_program/subsystem2/m2_make_recipes.txt`
- `app_program/subsystem2/motion_api.h`
- `app_program/subsystem2/motion_bus.c`
- `app_program/subsystem2/motion_bus.h`
- `app_program/subsystem2/motion_config.c`
- `app_program/subsystem2/motion_config.h`
- `app_program/subsystem2/motion_estimator.c`
- `app_program/subsystem2/motion_estimator.h`
- `app_program/subsystem2/motion_foundation_test.c`
- `app_program/subsystem2/motion_foundation_test.h`
- `app_program/subsystem2/motion_pid.c`
- `app_program/subsystem2/motion_pid.h`
- `app_program/subsystem2/motion_quad.c`
- `app_program/subsystem2/motion_quad.h`
- `app_program/subsystem2/motion_run.c`
- `app_program/subsystem2/motion_run.h`
- `app_program/subsystem2/motion_types.h`
- `app_program/subsystem2/move_plan.c`
- `app_program/subsystem2/move_plan.h`
- `app_program/subsystem2/subsystem2.c`
- `app_program/subsystem2/subsystem2.h`
- `app_program/subsystem2/subsystem2_test.c`
- `app_program/subsystem2/task_motion.c`
- `app_program/subsystem2/task_motion.h`
- `app_program/subsystem2/test_drive.c`
- `app_program/subsystem2/test_drive.h`
- `app_program/subsystem2/test_motor.c`
- `app_program/subsystem2/test_motor.h`
- `app_program/subsystem2/verify_motion.py`
- `app_program/subsystem2/wheel_loop.c`
- `app_program/subsystem2/wheel_loop.h`
- `docs/buddy2/MOTION_FOUNDATION_VERIFICATION.md`
- `docs/buddy2/MOTION_SYSTEM.md`
- `docs/buddy2/MOTOR_DRIVER.md`
- `testing/subsystem2/TEST_LOG.md`
- `testing/subsystem2/logs/2026-10-14_evidence_manifest.md`
- `testing/subsystem2/logs/2026-10-14_m2_arm_compile.txt`
- `testing/subsystem2/logs/2026-10-14_m2_host_default.txt`
- `testing/subsystem2/logs/2026-10-14_m2_host_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_m2_make_recipes.txt`
- `testing/subsystem2/logs/2026-10-14_motion_ownership.txt`
- `testing/subsystem2/logs/2026-10-14_pr_preflight_manifest.md`
- `testing/subsystem2/logs/2026-10-14_precommit20261014a_default.txt`
- `testing/subsystem2/logs/2026-10-14_precommit20261014a_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M3_default.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M3_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M4_default.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M4_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M5_default.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M5_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M6_default.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M6_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M7_arm.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M7_default.txt`
- `testing/subsystem2/logs/2026-10-14_verify_M7_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_verify_final_arm.txt`
- `testing/subsystem2/logs/2026-10-14_verify_final_default.txt`
- `testing/subsystem2/logs/2026-10-14_verify_final_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_verify_final_stack.txt`
- `testing/subsystem2/logs/2026-10-14_verify_strict_arm.txt`

## Source and script hashes

| File | SHA256 |
|---|---|
| `drv_encoder.c` | `f92daf16ac1943b39fd29e22cd6f63c35a3043e875472b7e1aaeecd420e2ef3e` |
| `drv_encoder.h` | `caa9f52de95f0710c6ca86f1e6779171d3dbadc40e447b3c08d6b9b0750ba235` |
| `drv_motor.c` | `1fd72f7b37fe5c807c529f96b34dee8a0d5aacc28e20fbc4149854cfa3e9a81d` |
| `drv_motor.h` | `7d0bad1946f3400e03cdc135b7c0d1b4bf63320e0f91a462fc6fe2fff8673e41` |
| `drv_rp2040.c` | `db14ea3c0428c60447775341f7aa2960d3b63be80f70d262331aa4bd1595eb40` |
| `drv_rp2040.h` | `ddb95b10f1b9af36a045e7a106992bbe876013d6bb61a1717dfde139ea1cdef8` |
| `enc_capture.c` | `586878820dee0bf82abc03b716be40727a9acee310ed4e09cbb9b9ba3bbccf87` |
| `enc_capture.h` | `3ee5ff131bd9aa8a545c67f92da8483a83ce878946fb618c132c99cda68e6670` |
| `motion_api.h` | `07ec7ef7c0613c5542dacadf43a1ec4fb4afe1cb3baf89ecc5a6a313c9cee484` |
| `motion_bus.c` | `19ad1ef597c806193ca5afff70ab0f8652fc4a47b89d4bad4782df608a282768` |
| `motion_bus.h` | `8491e984eb398be793d53eedab8bc6b154d36ed5f0d3dd5684b115448b5fa7eb` |
| `motion_config.c` | `6882d38a75107067a888c85649e858f3a962ca94fd24c5bf72d9965e1d7ec74f` |
| `motion_config.h` | `c877655d6f6264df6b2b4a4ed95dfa5e04f7f97020126cce751d6b277ad49731` |
| `motion_estimator.c` | `28f0524c23711b8abe5d31d8fe43ae4d84838fea812cfd10ff735c22fc998baf` |
| `motion_estimator.h` | `5dd22b79a02706f673331d1d522a051a5b7acd61e0a589146aa25aec4dbe78f9` |
| `motion_foundation_test.c` | `36056556a7324cc503892c8637833ce679c362ed7e4f98e62b0f8ca6abcc89d4` |
| `motion_foundation_test.h` | `2d0de8e7f9eaae6d6b58f08e100c5568c186b4ae53a774f18fb383165738002c` |
| `motion_pid.c` | `688c0edf5ad9ae5a68aee9aef8692dd026ed055e6a8d3c3ad0cb7c8fcd67d6e1` |
| `motion_pid.h` | `5f46af0f138580d6728d798067173f321dd33c4f7c00771b8051aba9fb4bfeb0` |
| `motion_quad.c` | `b6127f5392d1f7420988228c54457633375374ce86e613e9b3540e050a71613d` |
| `motion_quad.h` | `0f5e1757229802b7b5edd20a8b254a9b963b686a0a2b43cde8b553465b3d6972` |
| `motion_run.c` | `cd9239b966e8cbf60106a34cad5f349b2cf19a10fe6d6855ba6178be89b9d0c9` |
| `motion_run.h` | `5c04fb91b51d318acd9c992636b61194eeca3be8f251442ad88c110a88a41f42` |
| `motion_types.h` | `0a76753d2e16197188c2874ffdfea3fde8cb3a94053d93371a3bbefc9a649625` |
| `move_plan.c` | `727d78a41c066b142bfd92efeda157271e6da9aaa83289e514f7fd2d32ae7aaa` |
| `move_plan.h` | `35b0496acb1def9c86497ff59bbdbdf8138cbcd0c2c2847ec98ee95f274160c9` |
| `subsystem2.c` | `26b17efd7c8819c7830b643a588b85e18421e61fa6c39e4902714e7d3b907228` |
| `subsystem2.h` | `32659be92095df9e5f1d2ec280df1a25905c6b906b1c3a8faeafb728a64d0d2f` |
| `subsystem2_test.c` | `5d593125e20e92a25cc1f095a20dcab05e84ec6d60f57c616114f5195ba291d0` |
| `task_motion.c` | `6c4418c6f577c8fa04a782575fbcfc501e8c4f13e555c2253100c4ad929f58bc` |
| `task_motion.h` | `0776fa9b415fc07ed00b9258cbcbabcfd29e2df0256e7b3a3a9511ad61751bf0` |
| `task_producer.c` | `b8f1cabf4ffacbf95136e247a843fe60a06a2345503421b7283486b0b8006ba4` |
| `test_drive.c` | `7d1b563d2b1bc643b353f75238c94f4b5cc6a738e0128f3b206ff9c8f168c646` |
| `test_drive.h` | `9a83c11b8a5f11bec84f69555dd8582c2ab0a47799c4ec953ee53cdfc8f07256` |
| `test_motor.c` | `3895dcdedf0fafba016854404ea981550f43360bd27e8dca09c457ffc45bcdcb` |
| `test_motor.h` | `d1cb984f9fa1bf0d1c1418f2f0bc3e4c1094ff5a40af69e19f181a6438af6f04` |
| `verify_motion.py` | `8f16c303beacfe2e2115f6ff03de2c2a90290c911534c86672786d0632f4cc61` |
| `wheel_loop.c` | `7697b40c05693952a7015b7f2757926556b60e3fa95372c8425bf9285df9e141` |
| `wheel_loop.h` | `2e79492ffba21a7151f894658a30e10e5a19bf89c6fd58584e8de55046c45d34` |

## Commit-verified host rerun

This append updates the earlier preflight snapshot; historical results are
unchanged. Tested implementation: `50ee11ce345b86ce83559c47c0a2e8be055f6778`, branch `buddy2-motion-control-pr`.
The tracked tree was clean at execution start. Source/script content matches
the commit; inherited task_producer.c has checkout-only CRLF line endings.

Run start UTC: `2026-10-14T14:20:34.8278916Z`.
Run end UTC: `2026-10-14T14:20:48.1739628Z`.
Tester: Codex automated host execution. Compiler: GCC 14.2.0.

```sh
python -B app_program/subsystem2/verify_motion.py --style
python -B app_program/subsystem2/verify_motion.py --label commit50ee11c20261014a --analyze
```

Style subset: 17 files PASS. Strict C99 default and O2/fshort-enums profiles
with -fanalyzer: 95/95 PASS each, compiler and executable exits zero and no
warning diagnostics. S2-U01/U02 are excluded from the host runner. No full
firmware link, target RTOS execution or physical validation is established.

Fresh raw logs are embedded below to stay within the 69 authorised PR paths.
Bytes inside each raw-log code block are preserved exactly, identified by
length and SHA256. Temporary logs were removed only after archive verification.

CRLF-aware git diff --check found 33 existing blank-at-EOF exceptions in
preserved files. They were retained rather than cosmetically changing source
or evidence. The check with only blank-at-EOF disabled passed. Per-command
core.autocrlf=false preserves exact staged evidence bytes; Git configuration
and shared files were not changed.

### Committed host default

Original output: `verify_commit50ee11c20261014a_default.log`; bytes: 7649; SHA256: `76b3c98d2e3b908b2b139bc797086092149511efd5d6423f2d1913f8a90908d3`.

```text
C:\msys64\ucrt64\bin\gcc.exe -std=c99 -Wall -Wextra -Werror -pedantic -Wconversion -Wsign-conversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wdouble-promotion -Wundef -DMOTION_CONFIG_HOST_TEST -fanalyzer C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_encoder.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_rp2040.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\enc_capture.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_bus.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_config.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_estimator.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_foundation_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_pid.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_quad.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_run.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\move_plan.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\subsystem2_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\task_motion.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_drive.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\wheel_loop.c -o C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_default.exe
EXIT 0
C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_default.exe
[TEST] S2-U03 PASS  reset and zero configuration remain unconfigured
[TEST] S2-U04 PASS  null arguments are rejected
[TEST] S2-U05 PASS  both wheels require nonzero resolution and circumference
[TEST] S2-U06 PASS  unknown and invalid polarities are rejected
[TEST] S2-U07 PASS  zero track and nonpositive speed limits are rejected
[TEST] S2-U08 PASS  calibration evidence and enum states are checked
[TEST] S2-U09 PASS  asymmetric wheels and independent polarities are supported
[TEST] S2-U10 PASS  representation boundaries impose no invented calibration
[TEST] S2-U11 PASS  validation preserves every field on all nonnull paths
[TEST] S2-U12 PASS  unusual invalid enum values are rejected in every field
[TEST] subsystem2 M1A SUMMARY 10 passed, 0 failed
[TEST] S2-U13 PASS  quadrature directions, reversal and repeated states
[TEST] S2-U14 PASS  invalid jumps resync; diagnostics saturate
[TEST] S2-U15 PASS  decoder init, null, polarity and state validation
[TEST] S2-U16 PASS  signed count limits latch until explicit reset
[TEST] S2-U17 PASS  independent decoders apply polarity exactly once
[TEST] S2-U18 PASS  all sixteen state transitions match the convention
[TEST] S2-U19 PASS  distance, signed speed and actual variable elapsed time
[TEST] S2-U20 PASS  independent left/right wheel scales
[TEST] S2-U21 PASS  fractional distance, reversal and repeatability
[TEST] S2-U22 PASS  zero/backward/future/stale timing and fresh zero speed
[TEST] S2-U23 PASS  count, product, position and speed overflow rejection
[TEST] S2-U24 PASS  source generation and explicit odometry baseline reset
[TEST] S2-U25 PASS  invalid/unverified calibration and null estimation
[TEST] S2-U26 PASS  zero error and signed proportional response
[TEST] S2-U27 PASS  integral limits, variable elapsed time and reversal
[TEST] S2-U28 PASS  output saturation, anti-windup and recovery
[TEST] S2-U29 PASS  derivative on measurement avoids target kick
[TEST] S2-U30 PASS  reset, idle, stop and invalid timing clear dynamics
[TEST] S2-U31 PASS  invalid gains, bounds, confirmation and null arguments
[TEST] S2-U32 PASS  nonfinite arithmetic disables output safely
[TEST] S2-U33 PASS  independent PID state for both wheels
[TEST] S2-U34 PASS  caller edits do not alter copied calibration
[TEST] S2-U35 PASS  decoder to estimator to PID preserves vehicle sign
[TEST] S2-U36 PASS  pure-I large steps reach both output limits repeatedly
[TEST] S2-U37 PASS  integral crosses saturation and recovers from overspeed
[TEST] S2-U38 PASS  null updates invalidate cached speed and retain baseline
[TEST] S2-U39 PASS  nonzero fractional speeds and exact timing boundaries
[TEST] S2-U40 PASS  all PID bounds reject nonfinite and invalid values
[TEST] S2-U41 PASS  derivative, integral and output overflow disable PID
[TEST] S2-U42 PASS  asymmetric output headroom includes P/D and I bounds
[TEST] subsystem2 M1B-M1D SUMMARY 30 passed, 0 failed
[TEST] S2-U43 PASS  fake motor backend
[TEST] S2-U44 PASS  fake motor backend
[TEST] S2-U45 PASS  fake motor backend
[TEST] S2-U46 PASS  fake motor backend
[TEST] S2-U47 PASS  fake motor backend
[TEST] S2-U48 PASS  fake motor backend
[TEST] S2-U49 PASS  fake motor backend
[TEST] S2-U50 PASS  fake motor backend
[TEST] S2-U51 PASS  fake motor backend
[TEST] S2-U52 PASS  fake motor backend
[TEST] S2-U53 PASS  fake motor backend
[TEST] S2-U54 PASS  fake motor backend
[TEST] S2-U55 PASS  fake motor backend
[TEST] S2-U56 PASS  fake motor backend
[TEST] subsystem2 M2 SUMMARY 14 passed, 0 failed
[TEST] S2-U57 PASS motion integration
[TEST] S2-U58 PASS motion integration
[TEST] S2-U59 PASS motion integration
[TEST] S2-U60 PASS motion integration
[TEST] S2-U61 PASS motion integration
[TEST] S2-U62 PASS motion integration
[TEST] S2-U63 PASS motion integration
[TEST] S2-U64 PASS motion integration
[TEST] S2-U65 PASS motion integration
[TEST] S2-U66 PASS motion integration
[TEST] S2-U67 PASS motion integration
[TEST] S2-U68 PASS motion integration
[TEST] S2-U69 PASS motion integration
[TEST] S2-U70 PASS motion integration
[TEST] S2-U71 PASS motion integration
[TEST] S2-U72 PASS motion integration
[TEST] S2-U73 PASS motion integration
[TEST] S2-U74 PASS motion integration
[TEST] S2-U75 PASS motion integration
[TEST] S2-U76 PASS motion integration
[TEST] S2-U77 PASS motion integration
[TEST] S2-U78 PASS motion integration
[TEST] S2-U79 PASS motion integration
[TEST] S2-U80 PASS motion integration
[TEST] S2-U81 PASS motion integration
[TEST] S2-U82 PASS motion integration
[TEST] S2-U83 PASS motion integration
[TEST] S2-U84 PASS motion integration
[TEST] S2-U85 PASS motion integration
[TEST] S2-U86 PASS motion integration
[TEST] S2-U87 PASS motion integration
[TEST] S2-U88 PASS motion integration
[TEST] S2-U89 PASS motion integration
[TEST] S2-U90 PASS motion integration
[TEST] S2-U91 PASS motion integration
[TEST] S2-U92 PASS motion integration
[TEST] S2-U93 PASS motion integration
[TEST] S2-U94 PASS motion integration
[TEST] S2-U95 PASS motion integration
[TEST] S2-U96 PASS motion integration
[TEST] S2-U97 PASS motion integration
EXIT 0
```

### Committed host optimized

Original output: `verify_commit50ee11c20261014a_optimized.log`; bytes: 7671; SHA256: `b934e53f1a84ebdfb7a9aef09f601e5cbb8d6a6353fd04a3afc3667861e736f5`.

```text
C:\msys64\ucrt64\bin\gcc.exe -std=c99 -Wall -Wextra -Werror -pedantic -Wconversion -Wsign-conversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wdouble-promotion -Wundef -DMOTION_CONFIG_HOST_TEST -fanalyzer -O2 -fshort-enums C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_encoder.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_rp2040.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\enc_capture.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_bus.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_config.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_estimator.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_foundation_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_pid.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_quad.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_run.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\move_plan.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\subsystem2_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\task_motion.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_drive.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\wheel_loop.c -o C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_optimized.exe
EXIT 0
C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_optimized.exe
[TEST] S2-U03 PASS  reset and zero configuration remain unconfigured
[TEST] S2-U04 PASS  null arguments are rejected
[TEST] S2-U05 PASS  both wheels require nonzero resolution and circumference
[TEST] S2-U06 PASS  unknown and invalid polarities are rejected
[TEST] S2-U07 PASS  zero track and nonpositive speed limits are rejected
[TEST] S2-U08 PASS  calibration evidence and enum states are checked
[TEST] S2-U09 PASS  asymmetric wheels and independent polarities are supported
[TEST] S2-U10 PASS  representation boundaries impose no invented calibration
[TEST] S2-U11 PASS  validation preserves every field on all nonnull paths
[TEST] S2-U12 PASS  unusual invalid enum values are rejected in every field
[TEST] subsystem2 M1A SUMMARY 10 passed, 0 failed
[TEST] S2-U13 PASS  quadrature directions, reversal and repeated states
[TEST] S2-U14 PASS  invalid jumps resync; diagnostics saturate
[TEST] S2-U15 PASS  decoder init, null, polarity and state validation
[TEST] S2-U16 PASS  signed count limits latch until explicit reset
[TEST] S2-U17 PASS  independent decoders apply polarity exactly once
[TEST] S2-U18 PASS  all sixteen state transitions match the convention
[TEST] S2-U19 PASS  distance, signed speed and actual variable elapsed time
[TEST] S2-U20 PASS  independent left/right wheel scales
[TEST] S2-U21 PASS  fractional distance, reversal and repeatability
[TEST] S2-U22 PASS  zero/backward/future/stale timing and fresh zero speed
[TEST] S2-U23 PASS  count, product, position and speed overflow rejection
[TEST] S2-U24 PASS  source generation and explicit odometry baseline reset
[TEST] S2-U25 PASS  invalid/unverified calibration and null estimation
[TEST] S2-U26 PASS  zero error and signed proportional response
[TEST] S2-U27 PASS  integral limits, variable elapsed time and reversal
[TEST] S2-U28 PASS  output saturation, anti-windup and recovery
[TEST] S2-U29 PASS  derivative on measurement avoids target kick
[TEST] S2-U30 PASS  reset, idle, stop and invalid timing clear dynamics
[TEST] S2-U31 PASS  invalid gains, bounds, confirmation and null arguments
[TEST] S2-U32 PASS  nonfinite arithmetic disables output safely
[TEST] S2-U33 PASS  independent PID state for both wheels
[TEST] S2-U34 PASS  caller edits do not alter copied calibration
[TEST] S2-U35 PASS  decoder to estimator to PID preserves vehicle sign
[TEST] S2-U36 PASS  pure-I large steps reach both output limits repeatedly
[TEST] S2-U37 PASS  integral crosses saturation and recovers from overspeed
[TEST] S2-U38 PASS  null updates invalidate cached speed and retain baseline
[TEST] S2-U39 PASS  nonzero fractional speeds and exact timing boundaries
[TEST] S2-U40 PASS  all PID bounds reject nonfinite and invalid values
[TEST] S2-U41 PASS  derivative, integral and output overflow disable PID
[TEST] S2-U42 PASS  asymmetric output headroom includes P/D and I bounds
[TEST] subsystem2 M1B-M1D SUMMARY 30 passed, 0 failed
[TEST] S2-U43 PASS  fake motor backend
[TEST] S2-U44 PASS  fake motor backend
[TEST] S2-U45 PASS  fake motor backend
[TEST] S2-U46 PASS  fake motor backend
[TEST] S2-U47 PASS  fake motor backend
[TEST] S2-U48 PASS  fake motor backend
[TEST] S2-U49 PASS  fake motor backend
[TEST] S2-U50 PASS  fake motor backend
[TEST] S2-U51 PASS  fake motor backend
[TEST] S2-U52 PASS  fake motor backend
[TEST] S2-U53 PASS  fake motor backend
[TEST] S2-U54 PASS  fake motor backend
[TEST] S2-U55 PASS  fake motor backend
[TEST] S2-U56 PASS  fake motor backend
[TEST] subsystem2 M2 SUMMARY 14 passed, 0 failed
[TEST] S2-U57 PASS motion integration
[TEST] S2-U58 PASS motion integration
[TEST] S2-U59 PASS motion integration
[TEST] S2-U60 PASS motion integration
[TEST] S2-U61 PASS motion integration
[TEST] S2-U62 PASS motion integration
[TEST] S2-U63 PASS motion integration
[TEST] S2-U64 PASS motion integration
[TEST] S2-U65 PASS motion integration
[TEST] S2-U66 PASS motion integration
[TEST] S2-U67 PASS motion integration
[TEST] S2-U68 PASS motion integration
[TEST] S2-U69 PASS motion integration
[TEST] S2-U70 PASS motion integration
[TEST] S2-U71 PASS motion integration
[TEST] S2-U72 PASS motion integration
[TEST] S2-U73 PASS motion integration
[TEST] S2-U74 PASS motion integration
[TEST] S2-U75 PASS motion integration
[TEST] S2-U76 PASS motion integration
[TEST] S2-U77 PASS motion integration
[TEST] S2-U78 PASS motion integration
[TEST] S2-U79 PASS motion integration
[TEST] S2-U80 PASS motion integration
[TEST] S2-U81 PASS motion integration
[TEST] S2-U82 PASS motion integration
[TEST] S2-U83 PASS motion integration
[TEST] S2-U84 PASS motion integration
[TEST] S2-U85 PASS motion integration
[TEST] S2-U86 PASS motion integration
[TEST] S2-U87 PASS motion integration
[TEST] S2-U88 PASS motion integration
[TEST] S2-U89 PASS motion integration
[TEST] S2-U90 PASS motion integration
[TEST] S2-U91 PASS motion integration
[TEST] S2-U92 PASS motion integration
[TEST] S2-U93 PASS motion integration
[TEST] S2-U94 PASS motion integration
[TEST] S2-U95 PASS motion integration
[TEST] S2-U96 PASS motion integration
[TEST] S2-U97 PASS motion integration
EXIT 0
```

## Final organisation verification

Run start UTC: `2026-10-14T14:25:36.3181209Z`; end UTC:
`2026-10-14T14:25:49.5483297Z`. Tested commit: `143a3aa2a0a00d5c5038d15f8b97423b5b7ec7f7`.
Tracked files were clean at run start; no production/test/script content changed.

```sh
python -B app_program/subsystem2/verify_motion.py --style
python -B app_program/subsystem2/verify_motion.py --label organisation143a3aa20261014b --analyze
```

Strict GCC C99 default and O2/fshort-enums profiles with -fanalyzer each passed
95/95 groups (S2-U03–U97), no compiler warnings. Style subset passed 17 files.
S2-U01/U02 remain excluded from host execution. 97 stable definitions, the
repaired definition table, 1,650 live links, flat Make discovery and TEST=1
registration were checked. No firmware build, target or hardware test occurred.

Only generated executables and redundant temporary log copies from this run
were removed, after embedding and verifying their original log bytes below.
No Git add, commit, push, fetch, merge, branch switch or history operation was
performed in this final organisation pass. This evidence append is uncommitted.

### Organisation host default

Original output: `verify_organisation143a3aa20261014b_default.log`; bytes: 7649; SHA256: `76b3c98d2e3b908b2b139bc797086092149511efd5d6423f2d1913f8a90908d3`.

```text
C:\msys64\ucrt64\bin\gcc.exe -std=c99 -Wall -Wextra -Werror -pedantic -Wconversion -Wsign-conversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wdouble-promotion -Wundef -DMOTION_CONFIG_HOST_TEST -fanalyzer C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_encoder.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_rp2040.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\enc_capture.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_bus.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_config.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_estimator.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_foundation_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_pid.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_quad.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_run.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\move_plan.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\subsystem2_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\task_motion.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_drive.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\wheel_loop.c -o C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_default.exe
EXIT 0
C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_default.exe
[TEST] S2-U03 PASS  reset and zero configuration remain unconfigured
[TEST] S2-U04 PASS  null arguments are rejected
[TEST] S2-U05 PASS  both wheels require nonzero resolution and circumference
[TEST] S2-U06 PASS  unknown and invalid polarities are rejected
[TEST] S2-U07 PASS  zero track and nonpositive speed limits are rejected
[TEST] S2-U08 PASS  calibration evidence and enum states are checked
[TEST] S2-U09 PASS  asymmetric wheels and independent polarities are supported
[TEST] S2-U10 PASS  representation boundaries impose no invented calibration
[TEST] S2-U11 PASS  validation preserves every field on all nonnull paths
[TEST] S2-U12 PASS  unusual invalid enum values are rejected in every field
[TEST] subsystem2 M1A SUMMARY 10 passed, 0 failed
[TEST] S2-U13 PASS  quadrature directions, reversal and repeated states
[TEST] S2-U14 PASS  invalid jumps resync; diagnostics saturate
[TEST] S2-U15 PASS  decoder init, null, polarity and state validation
[TEST] S2-U16 PASS  signed count limits latch until explicit reset
[TEST] S2-U17 PASS  independent decoders apply polarity exactly once
[TEST] S2-U18 PASS  all sixteen state transitions match the convention
[TEST] S2-U19 PASS  distance, signed speed and actual variable elapsed time
[TEST] S2-U20 PASS  independent left/right wheel scales
[TEST] S2-U21 PASS  fractional distance, reversal and repeatability
[TEST] S2-U22 PASS  zero/backward/future/stale timing and fresh zero speed
[TEST] S2-U23 PASS  count, product, position and speed overflow rejection
[TEST] S2-U24 PASS  source generation and explicit odometry baseline reset
[TEST] S2-U25 PASS  invalid/unverified calibration and null estimation
[TEST] S2-U26 PASS  zero error and signed proportional response
[TEST] S2-U27 PASS  integral limits, variable elapsed time and reversal
[TEST] S2-U28 PASS  output saturation, anti-windup and recovery
[TEST] S2-U29 PASS  derivative on measurement avoids target kick
[TEST] S2-U30 PASS  reset, idle, stop and invalid timing clear dynamics
[TEST] S2-U31 PASS  invalid gains, bounds, confirmation and null arguments
[TEST] S2-U32 PASS  nonfinite arithmetic disables output safely
[TEST] S2-U33 PASS  independent PID state for both wheels
[TEST] S2-U34 PASS  caller edits do not alter copied calibration
[TEST] S2-U35 PASS  decoder to estimator to PID preserves vehicle sign
[TEST] S2-U36 PASS  pure-I large steps reach both output limits repeatedly
[TEST] S2-U37 PASS  integral crosses saturation and recovers from overspeed
[TEST] S2-U38 PASS  null updates invalidate cached speed and retain baseline
[TEST] S2-U39 PASS  nonzero fractional speeds and exact timing boundaries
[TEST] S2-U40 PASS  all PID bounds reject nonfinite and invalid values
[TEST] S2-U41 PASS  derivative, integral and output overflow disable PID
[TEST] S2-U42 PASS  asymmetric output headroom includes P/D and I bounds
[TEST] subsystem2 M1B-M1D SUMMARY 30 passed, 0 failed
[TEST] S2-U43 PASS  fake motor backend
[TEST] S2-U44 PASS  fake motor backend
[TEST] S2-U45 PASS  fake motor backend
[TEST] S2-U46 PASS  fake motor backend
[TEST] S2-U47 PASS  fake motor backend
[TEST] S2-U48 PASS  fake motor backend
[TEST] S2-U49 PASS  fake motor backend
[TEST] S2-U50 PASS  fake motor backend
[TEST] S2-U51 PASS  fake motor backend
[TEST] S2-U52 PASS  fake motor backend
[TEST] S2-U53 PASS  fake motor backend
[TEST] S2-U54 PASS  fake motor backend
[TEST] S2-U55 PASS  fake motor backend
[TEST] S2-U56 PASS  fake motor backend
[TEST] subsystem2 M2 SUMMARY 14 passed, 0 failed
[TEST] S2-U57 PASS motion integration
[TEST] S2-U58 PASS motion integration
[TEST] S2-U59 PASS motion integration
[TEST] S2-U60 PASS motion integration
[TEST] S2-U61 PASS motion integration
[TEST] S2-U62 PASS motion integration
[TEST] S2-U63 PASS motion integration
[TEST] S2-U64 PASS motion integration
[TEST] S2-U65 PASS motion integration
[TEST] S2-U66 PASS motion integration
[TEST] S2-U67 PASS motion integration
[TEST] S2-U68 PASS motion integration
[TEST] S2-U69 PASS motion integration
[TEST] S2-U70 PASS motion integration
[TEST] S2-U71 PASS motion integration
[TEST] S2-U72 PASS motion integration
[TEST] S2-U73 PASS motion integration
[TEST] S2-U74 PASS motion integration
[TEST] S2-U75 PASS motion integration
[TEST] S2-U76 PASS motion integration
[TEST] S2-U77 PASS motion integration
[TEST] S2-U78 PASS motion integration
[TEST] S2-U79 PASS motion integration
[TEST] S2-U80 PASS motion integration
[TEST] S2-U81 PASS motion integration
[TEST] S2-U82 PASS motion integration
[TEST] S2-U83 PASS motion integration
[TEST] S2-U84 PASS motion integration
[TEST] S2-U85 PASS motion integration
[TEST] S2-U86 PASS motion integration
[TEST] S2-U87 PASS motion integration
[TEST] S2-U88 PASS motion integration
[TEST] S2-U89 PASS motion integration
[TEST] S2-U90 PASS motion integration
[TEST] S2-U91 PASS motion integration
[TEST] S2-U92 PASS motion integration
[TEST] S2-U93 PASS motion integration
[TEST] S2-U94 PASS motion integration
[TEST] S2-U95 PASS motion integration
[TEST] S2-U96 PASS motion integration
[TEST] S2-U97 PASS motion integration
EXIT 0
```

### Organisation host optimized

Original output: `verify_organisation143a3aa20261014b_optimized.log`; bytes: 7671; SHA256: `b934e53f1a84ebdfb7a9aef09f601e5cbb8d6a6353fd04a3afc3667861e736f5`.

```text
C:\msys64\ucrt64\bin\gcc.exe -std=c99 -Wall -Wextra -Werror -pedantic -Wconversion -Wsign-conversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wdouble-promotion -Wundef -DMOTION_CONFIG_HOST_TEST -fanalyzer -O2 -fshort-enums C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_encoder.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\drv_rp2040.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\enc_capture.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_bus.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_config.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_estimator.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_foundation_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_pid.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_quad.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\motion_run.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\move_plan.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\subsystem2_test.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\task_motion.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_drive.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\test_motor.c C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\wheel_loop.c -o C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_optimized.exe
EXIT 0
C:\Users\Hp\.codex\worktrees\buddy2-motion-control-pr\INF2004-Embedded-Systems\app_program\subsystem2\verify_generated_optimized.exe
[TEST] S2-U03 PASS  reset and zero configuration remain unconfigured
[TEST] S2-U04 PASS  null arguments are rejected
[TEST] S2-U05 PASS  both wheels require nonzero resolution and circumference
[TEST] S2-U06 PASS  unknown and invalid polarities are rejected
[TEST] S2-U07 PASS  zero track and nonpositive speed limits are rejected
[TEST] S2-U08 PASS  calibration evidence and enum states are checked
[TEST] S2-U09 PASS  asymmetric wheels and independent polarities are supported
[TEST] S2-U10 PASS  representation boundaries impose no invented calibration
[TEST] S2-U11 PASS  validation preserves every field on all nonnull paths
[TEST] S2-U12 PASS  unusual invalid enum values are rejected in every field
[TEST] subsystem2 M1A SUMMARY 10 passed, 0 failed
[TEST] S2-U13 PASS  quadrature directions, reversal and repeated states
[TEST] S2-U14 PASS  invalid jumps resync; diagnostics saturate
[TEST] S2-U15 PASS  decoder init, null, polarity and state validation
[TEST] S2-U16 PASS  signed count limits latch until explicit reset
[TEST] S2-U17 PASS  independent decoders apply polarity exactly once
[TEST] S2-U18 PASS  all sixteen state transitions match the convention
[TEST] S2-U19 PASS  distance, signed speed and actual variable elapsed time
[TEST] S2-U20 PASS  independent left/right wheel scales
[TEST] S2-U21 PASS  fractional distance, reversal and repeatability
[TEST] S2-U22 PASS  zero/backward/future/stale timing and fresh zero speed
[TEST] S2-U23 PASS  count, product, position and speed overflow rejection
[TEST] S2-U24 PASS  source generation and explicit odometry baseline reset
[TEST] S2-U25 PASS  invalid/unverified calibration and null estimation
[TEST] S2-U26 PASS  zero error and signed proportional response
[TEST] S2-U27 PASS  integral limits, variable elapsed time and reversal
[TEST] S2-U28 PASS  output saturation, anti-windup and recovery
[TEST] S2-U29 PASS  derivative on measurement avoids target kick
[TEST] S2-U30 PASS  reset, idle, stop and invalid timing clear dynamics
[TEST] S2-U31 PASS  invalid gains, bounds, confirmation and null arguments
[TEST] S2-U32 PASS  nonfinite arithmetic disables output safely
[TEST] S2-U33 PASS  independent PID state for both wheels
[TEST] S2-U34 PASS  caller edits do not alter copied calibration
[TEST] S2-U35 PASS  decoder to estimator to PID preserves vehicle sign
[TEST] S2-U36 PASS  pure-I large steps reach both output limits repeatedly
[TEST] S2-U37 PASS  integral crosses saturation and recovers from overspeed
[TEST] S2-U38 PASS  null updates invalidate cached speed and retain baseline
[TEST] S2-U39 PASS  nonzero fractional speeds and exact timing boundaries
[TEST] S2-U40 PASS  all PID bounds reject nonfinite and invalid values
[TEST] S2-U41 PASS  derivative, integral and output overflow disable PID
[TEST] S2-U42 PASS  asymmetric output headroom includes P/D and I bounds
[TEST] subsystem2 M1B-M1D SUMMARY 30 passed, 0 failed
[TEST] S2-U43 PASS  fake motor backend
[TEST] S2-U44 PASS  fake motor backend
[TEST] S2-U45 PASS  fake motor backend
[TEST] S2-U46 PASS  fake motor backend
[TEST] S2-U47 PASS  fake motor backend
[TEST] S2-U48 PASS  fake motor backend
[TEST] S2-U49 PASS  fake motor backend
[TEST] S2-U50 PASS  fake motor backend
[TEST] S2-U51 PASS  fake motor backend
[TEST] S2-U52 PASS  fake motor backend
[TEST] S2-U53 PASS  fake motor backend
[TEST] S2-U54 PASS  fake motor backend
[TEST] S2-U55 PASS  fake motor backend
[TEST] S2-U56 PASS  fake motor backend
[TEST] subsystem2 M2 SUMMARY 14 passed, 0 failed
[TEST] S2-U57 PASS motion integration
[TEST] S2-U58 PASS motion integration
[TEST] S2-U59 PASS motion integration
[TEST] S2-U60 PASS motion integration
[TEST] S2-U61 PASS motion integration
[TEST] S2-U62 PASS motion integration
[TEST] S2-U63 PASS motion integration
[TEST] S2-U64 PASS motion integration
[TEST] S2-U65 PASS motion integration
[TEST] S2-U66 PASS motion integration
[TEST] S2-U67 PASS motion integration
[TEST] S2-U68 PASS motion integration
[TEST] S2-U69 PASS motion integration
[TEST] S2-U70 PASS motion integration
[TEST] S2-U71 PASS motion integration
[TEST] S2-U72 PASS motion integration
[TEST] S2-U73 PASS motion integration
[TEST] S2-U74 PASS motion integration
[TEST] S2-U75 PASS motion integration
[TEST] S2-U76 PASS motion integration
[TEST] S2-U77 PASS motion integration
[TEST] S2-U78 PASS motion integration
[TEST] S2-U79 PASS motion integration
[TEST] S2-U80 PASS motion integration
[TEST] S2-U81 PASS motion integration
[TEST] S2-U82 PASS motion integration
[TEST] S2-U83 PASS motion integration
[TEST] S2-U84 PASS motion integration
[TEST] S2-U85 PASS motion integration
[TEST] S2-U86 PASS motion integration
[TEST] S2-U87 PASS motion integration
[TEST] S2-U88 PASS motion integration
[TEST] S2-U89 PASS motion integration
[TEST] S2-U90 PASS motion integration
[TEST] S2-U91 PASS motion integration
[TEST] S2-U92 PASS motion integration
[TEST] S2-U93 PASS motion integration
[TEST] S2-U94 PASS motion integration
[TEST] S2-U95 PASS motion integration
[TEST] S2-U96 PASS motion integration
[TEST] S2-U97 PASS motion integration
EXIT 0
```

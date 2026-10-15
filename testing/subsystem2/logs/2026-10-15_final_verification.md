# Buddy 2 normal-repository finalisation — 2026-10-15

Repository: `C:/Users/Hp/Documents/GitHub/INF2004-Embedded-Systems`.
Branch: `buddy2-Motor-and-encoder-control`. Base HEAD: `0482b352c4d30e8e95831ae8ff931c8952dbfeb9`.
Recorded Singapore time: 2026-10-15T12:55:50.011554+08:00.

## Revision and preservation

The current organisation/docs/evidence are an uncommitted working tree. These
new runs are WORKING-TREE results, not claims of a new committed revision.
Production C/H, test C/H, verify_motion.py and m2_make_recipes.txt are unchanged
from starting HEAD. All 40 match the archived worktree implementation apart
from line endings; no older implementation overwrote newer user code.
The old worktree was already archived before this request; snapshot
7215e6055c9539535e19c434433c398a7750ba5e was read without restoring a worktree.

Three technical documents were moved to docs/buddy2; a concise source README
was added; test-table/live links repaired. Later genuine test records and three
missing evidence files were recovered from the archive. Existing logs/failures
were not replaced. The archived TEST_LOG prefix was compared before importing
its missing suffix; no divergent historical result was overwritten.

## Executed verification

```sh
python -B app_program/subsystem2/verify_motion.py --style
python -B app_program/subsystem2/verify_motion.py --label finalnormal20261015a --analyze
python -B app_program/subsystem2/verify_motion.py --label finalnormal20261015a --arm --strict
python -B app_program/subsystem2/verify_motion.py --label finalnormal20261015b --arm --strict
```

First batch start: 2026-10-15T04:50:59.7102842Z (12:50:59 Singapore).
Separate host end time was not captured. Host GCC 14.2.0 strict C99 default
and O2/fshort-enums profiles with -fanalyzer each passed all 95 groups,
S2-U03–U97; compiler and executable exits zero, no warning diagnostics.
Style subset passed 17 files. All 97 stable definitions are preserved.

Initial ARM launch failed in the sandbox with Windows error 623, Illegal
System DLL Relocation, before any compiler invocation was recorded. Its opened
ARM log was zero bytes with no EXIT markers; that empty disposable file is
removed, not represented as a compiler PASS or reconstructed failure log.
The exception was reported by the execution tool. The separately saved retry
transcript preserves the actual successful retry commands, times and exit.
No project/toolchain changes were made to work around the launch environment.

ARM GNU 10.3.1 retry passed 76 object compilations: 19 C files each for
SMP=0/1 and APP_TEST=0/1. Strict warnings apply to new application units, with
supplied headers isolated as system headers; inherited demo/test units retain
template flags. Zero compiler warnings/errors occurred. This is compiler
verification using recorded Make flags, not an integrated Make build/link.

Read-only arm-none-eabi-nm inspection of the final 19-object TEST=1/SMP=1 set
found 65 strong definitions, no duplicate strong symbols, no unresolved Buddy 2
internal symbol and no firmware main symbol. Kernel/vendor dependencies are
not resolved by that inspection; full linking is still required.

The installed SDK is 1.5.1, whereas README documents 2.2.0 (and separately
reports 2.3.0 tested). PICO_SDK_PATH is unset in this environment. Make's USB
include paths target the 2.x layout; installed 1.5.1 has pico.h under
src/common/pico_base/include instead of the expected platform/header layout.
Prior genuine build_make logs record usb_sdk_compat.c:6 fatal pico.h not found.
No new full Make build is attempted without a compatible SDK; no Makefile,
SDK or shared configuration is altered. No linked ELF/UF2 or full size is
claimed. No target tests, flashing or physical motor actuation occurred.

S2-U01/U02 remain unverified on target. Host tests do not validate RTOS object
startup failures, real GPIO dispatcher timing, SMP ISR contention, stack margins
or emergency response latency. Existing U90 tests early wakes in READY rather
than ACTIVE; active-loop timing needs additional target/integration coverage.
Barr-C review retains documented float/vendor-type/MMIO conventions and
inherited template formatting. It is not certified standards compliance.
No confirmed Critical/High Buddy 2 software defect was identified.

## Evidence integrity

| File | Bytes | SHA256 |
|---|---:|---|
| [2026-10-15_finalnormal_host_default.txt](2026-10-15_finalnormal_host_default.txt) | 7174 | `531d53ede3c68e9d7d4f22388ff50f60928b6ac893073790e3e9f4198c45f89d` |
| [2026-10-15_finalnormal_host_optimized.txt](2026-10-15_finalnormal_host_optimized.txt) | 7196 | `de478b08a16f6e937ca7161c8c39276c4b2f5a21adddc85ebbcfc53095a5cd3a` |
| [2026-10-15_finalnormal_arm_strict.txt](2026-10-15_finalnormal_arm_strict.txt) | 377688 | `2d5b855d0e4a62c4257096880aba30c07d837ea5f19d3f3529c8558fab0c11b0` |
| [2026-10-15_arm_retry_transcript.txt](2026-10-15_arm_retry_transcript.txt) | 366 | `cf0e72061e86e14192afead1734f858b55c280372182a3d3af57e570cc39517d` |

## Source/script/recipe snapshot

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
| `m2_make_recipes.txt` | `99bad487374dc95052f72497c86af38ba450c77472489a230c1ccfc979e9f0c9` |
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

## Integration and manual merge classification

READY FOR MANUAL COMMIT/PUSH: the reviewed Buddy 2 working changes and records.
BLOCKED FOR DIRECT MERGE AS-IS: the original branch includes committed root
AGENTS.md, eight reference_docs files and six build_make verification logs
outside the Buddy 2 scope. Latest locally available main is
2886fa8a0118185674f59964ab832d7c4e3ceabb; no fetch was performed and remote
freshness is not asserted. No history rewrite or Git mutation is performed.

Producer retirement remains coordinated integration work: Buddy 3's started
consumer blocks on its samples, and Buddy 4 uses the shared demonstration
statistics and pool/semaphore. Removing the Buddy 2 chain alone would break
those dependencies. The real motion task is already started independently.

Controller arbitration, provisional API agreement (including Week 6 raw-count
telemetry absent from current status), shared GPIO dispatcher, pin/PWM claims,
priority/stack review, calibration, physical validation and full linking remain.
See [current handoff](../../../docs/buddy2/MOTION_SYSTEM.md#normal-repository-handoff--2026-10-15).

## Final filesystem and ownership audit

- 719 files outside the authorised Buddy 2 folders remain byte-identical.
- All 38 C/H files, verification script and recipe remain byte-identical to the starting normal repository.
- Original log-folder files retain their bytes; 63 newly generated temporary files were removed after evidence verification.
- 1,857 local documentation/evidence links resolve; all 97 test definitions remain unique.
- Tracked git diff --check and whitespace checks of every untracked file passed.
- Branch and HEAD remain unchanged; the index is empty. No Git mutation was performed.
- Exactly 16 filesystem paths changed (moves appear as a source deletion plus destination addition before staging).

### Exact changed-file inventory

- `app_program/subsystem2/README.md`
- `docs/buddy2/MOTION_FOUNDATION_VERIFICATION.md`
- `docs/buddy2/MOTION_SYSTEM.md`
- `docs/buddy2/MOTOR_DRIVER.md`
- `testing/subsystem2/TEST_LOG.md`
- `testing/subsystem2/logs/2026-10-14_precommit20261014a_default.txt`
- `testing/subsystem2/logs/2026-10-14_precommit20261014a_optimized.txt`
- `testing/subsystem2/logs/2026-10-14_pr_preflight_manifest.md`
- `testing/subsystem2/logs/2026-10-15_arm_retry_transcript.txt`
- `testing/subsystem2/logs/2026-10-15_finalnormal_arm_strict.txt`
- `testing/subsystem2/logs/2026-10-15_finalnormal_host_default.txt`
- `testing/subsystem2/logs/2026-10-15_finalnormal_host_optimized.txt`
- `testing/subsystem2/logs/2026-10-15_final_verification.md`
- `app_program/subsystem2/MOTION_FOUNDATION_VERIFICATION.md`
- `app_program/subsystem2/MOTION_SYSTEM.md`
- `app_program/subsystem2/MOTOR_DRIVER.md`

### Final directory inventory

```text
app_program/subsystem2/
  README.md
  drv_encoder.c
  drv_encoder.h
  drv_motor.c
  drv_motor.h
  drv_rp2040.c
  drv_rp2040.h
  enc_capture.c
  enc_capture.h
  m2_make_recipes.txt
  motion_api.h
  motion_bus.c
  motion_bus.h
  motion_config.c
  motion_config.h
  motion_estimator.c
  motion_estimator.h
  motion_foundation_test.c
  motion_foundation_test.h
  motion_pid.c
  motion_pid.h
  motion_quad.c
  motion_quad.h
  motion_run.c
  motion_run.h
  motion_types.h
  move_plan.c
  move_plan.h
  subsystem2.c
  subsystem2.h
  subsystem2_test.c
  task_motion.c
  task_motion.h
  task_producer.c
  test_drive.c
  test_drive.h
  test_motor.c
  test_motor.h
  verify_motion.py
  wheel_loop.c
  wheel_loop.h
docs/buddy2/
  MOTION_FOUNDATION_VERIFICATION.md
  MOTION_SYSTEM.md
  MOTOR_DRIVER.md
testing/subsystem2/
  TEST_LOG.md
  logs/ (31 files: preserved originals, recovered genuine evidence and fresh verification)
```

## Integrated ARM log relocation — 2026-10-15

This append supersedes the earlier scope-based BLOCKED classification above:
the user now explicitly approves retaining root AGENTS.md and reference_docs.
Those files are unchanged. The six previously committed build_make logs were
verified to be captured evidence, then copied to unique descriptive .txt names.
All six destinations were byte/hash-verified before any original was removed.
No original historical result or raw log content was edited or discarded.
No new build or test execution is claimed. The original evidence dates remain
2026-10-14; 2026-10-15 is the relocation date.

| Original repository path | Archived evidence | Bytes | Before/after SHA256 |
|---|---|---:|---|
| `build_make/buddy2_arm_verification_20261014_metadata.log` | [2026-10-14_integrated_arm_metadata.txt](2026-10-14_integrated_arm_metadata.txt) | 3465 | `69c3e807ec035778c23fe080092205f374f521cd49cb5e15b337a2761c55f247` |
| `build_make/buddy2_arm_verification_20261014_normal.log` | [2026-10-14_integrated_make_normal_sdk_failure.txt](2026-10-14_integrated_make_normal_sdk_failure.txt) | 235291 | `6a08ead18209236a78fcfe4f27bdda9446a511bcc3950ea6e8477de0e3e17b3c` |
| `build_make/buddy2_arm_verification_20261014_objects_normal.log` | [2026-10-14_integrated_arm_objects_normal.txt](2026-10-14_integrated_arm_objects_normal.txt) | 27539 | `23d6ba34070c08fee9f20c53c5b638da0168deecbf6e8ce54971deeb14f80cb9` |
| `build_make/buddy2_arm_verification_20261014_objects_test.log` | [2026-10-14_integrated_arm_objects_test.txt](2026-10-14_integrated_arm_objects_test.txt) | 27539 | `a6f2571c6c7fde8bef5a967434ca14d8dd3d385044a161471be230a79a83a1c3` |
| `build_make/buddy2_arm_verification_20261014_test.log` | [2026-10-14_integrated_make_test_missing_compiler.txt](2026-10-14_integrated_make_test_missing_compiler.txt) | 167 | `bcc1648cb7c00c4d8c221ef49c4cf10bc31a014d7d65fbf5ffb571593c8e921c` |
| `build_make/buddy2_arm_verification_20261014_test_retry.log` | [2026-10-14_integrated_make_test_sdk_failure.txt](2026-10-14_integrated_make_test_sdk_failure.txt) | 225775 | `4a538d89c60ed1b015e3dbf4ea88a8a7a7c0e13b8e745bb2b0a4d9d79289fc37` |

### Build dependencies and final scope

Inspection covered 79 Makefiles/build/verification scripts; none references
these log names or their common buddy2_arm_verification prefix. Make rules
depend on C/assembly sources, objects, generated .d dependencies, linker script
and SDK sources, not these logs. The Phase 11 USB verifier accepts an explicit
caller-supplied log argument and does not hardcode these filenames.
verify_motion.py still reads its unchanged m2_make_recipes.txt; neither file moved.
The missing optional build_make/sources.mk was already absent and is included
with -include; it has no relationship to these logs and was not changed.

`git diff main -- build_make` is empty after relocation.
`git diff main...HEAD -- build_make` still reports the six committed additions
because that comparison excludes uncommitted deletions; they disappear from
the final-tree diff once the user commits this relocation. No actual Makefile
or script was modified. Root AGENTS.md and reference_docs remain approved
final-tree additions against main. Other Buddy 2 working changes are preserved.

No build-system dependency prevents relocation. No staging, commit, push, fetch,
merge, branch switch, worktree creation or history rewrite occurred. Remaining
SDK linking, controller/GPIO integration, calibration and physical-validation
limitations are unchanged; this organisation pass does not resolve them.

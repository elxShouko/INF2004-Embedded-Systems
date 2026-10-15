# Buddy 2 evidence archive and cleanup manifest — 2026-10-14

This manifest records the authorised folder cleanup, not a test execution.
No compiler, host suite, firmware, target test or powered hardware procedure
was run during cleanup. No missing log or historical result was reconstructed.

## Cleanup revision and scope

- Repository: `C:/Users/Hp/Documents/GitHub/INF2004-Embedded-Systems`.
- Branch: `buddy2-Motor-and-encoder-control`.
- HEAD: `9e40a56d55955291c54957c459e02af1480bc31e`.
- Starting working tree already contained 3 modified C/H files and 36 untracked
  additions from development. These were preserved; cleanup does not claim
  that HEAD alone contains the tested or current implementation.
- Only 20 explicitly approved verification logs were relocated, plus one
  additional copy of the recipe. No C/H file or verification script changed.
- Documentation edits: `testing/subsystem2/TEST_LOG.md`,
  `app_program/subsystem2/MOTOR_DRIVER.md` and
  `app_program/subsystem2/MOTION_SYSTEM.md`. The latter two have necessary
  archive-link/location updates; prior outcomes are preserved.
- The original [Make recipe](../../../app_program/subsystem2/m2_make_recipes.txt)
  remains in the source folder because verify_motion.py reads it.
- No Git stage, commit, push, merge, rebase or branch switch occurred.
  No shared files, other testing folders or teammate sources were modified.

## Byte-preserving archive inventory

The source folder was
`C:/Users/Hp/Documents/GitHub/INF2004-Embedded-Systems/app_program/subsystem2/`.
The destination folder is
`C:/Users/Hp/Documents/GitHub/INF2004-Embedded-Systems/testing/subsystem2/logs/`.

Before relocating any original, all sources/destinations were preflighted,
destination collisions rejected, source bytes hashed, all 21 copies created,
then source and destination sizes/hashes rechecked. Only afterwards were the
20 source-folder .log copies removed to complete the approved moves. The
additional recipe copy was verified and its original retained. Final archive
hashes match the initial whole-repository snapshot. Original log bytes,
line endings, compiler diagnostics, absolute historical command paths and all
result lines are unchanged. Old paths inside raw logs are provenance, not
current file dependencies. Tracked status below is the ORIGINAL source status
before cleanup; no new archive was staged.

| Original repository path | Archived file | Operation | Original Git status | Bytes | Before SHA256 | Final archived SHA256 |
|---|---|---|---|---:|---|---|
| `app_program/subsystem2/m2_arm_compile.log` | [2026-10-14_m2_arm_compile.txt](2026-10-14_m2_arm_compile.txt) | Moved after verified copy | Tracked | 280720 | `39a7c4e70e35a5db5edaf4a6f9abae9b042e3806d028ed663fbda4133ac1958a` | `39a7c4e70e35a5db5edaf4a6f9abae9b042e3806d028ed663fbda4133ac1958a` |
| `app_program/subsystem2/m2_host_default.log` | [2026-10-14_m2_host_default.txt](2026-10-14_m2_host_default.txt) | Moved after verified copy | Tracked | 3558 | `349c5b49ac4640771513dc2b1afbfc6fcc715a943cc28835d4d400e4c6bfb2a7` | `349c5b49ac4640771513dc2b1afbfc6fcc715a943cc28835d4d400e4c6bfb2a7` |
| `app_program/subsystem2/m2_host_optimized.log` | [2026-10-14_m2_host_optimized.txt](2026-10-14_m2_host_optimized.txt) | Moved after verified copy | Tracked | 3558 | `349c5b49ac4640771513dc2b1afbfc6fcc715a943cc28835d4d400e4c6bfb2a7` | `349c5b49ac4640771513dc2b1afbfc6fcc715a943cc28835d4d400e4c6bfb2a7` |
| `app_program/subsystem2/MOTION_OWNERSHIP.log` | [2026-10-14_motion_ownership.txt](2026-10-14_motion_ownership.txt) | Moved after verified copy | Untracked | 6787 | `5da91e172bd7d000cb29bcf3aa8e6431a013cf3df7933e775694d1c5cbb2c0fc` | `5da91e172bd7d000cb29bcf3aa8e6431a013cf3df7933e775694d1c5cbb2c0fc` |
| `app_program/subsystem2/verify_M3_default.log` | [2026-10-14_verify_M3_default.txt](2026-10-14_verify_M3_default.txt) | Moved after verified copy | Untracked | 5310 | `827dbb7bacb5c84dd90502849f8dfadbb160ed6b7ba05278c77f52a273832656` | `827dbb7bacb5c84dd90502849f8dfadbb160ed6b7ba05278c77f52a273832656` |
| `app_program/subsystem2/verify_M3_optimized.log` | [2026-10-14_verify_M3_optimized.txt](2026-10-14_verify_M3_optimized.txt) | Moved after verified copy | Untracked | 5332 | `8990c87aa598b5d5e0dc151c7f8bc75cca5ef844951a7137137f9dd3fabae30f` | `8990c87aa598b5d5e0dc151c7f8bc75cca5ef844951a7137137f9dd3fabae30f` |
| `app_program/subsystem2/verify_M4_default.log` | [2026-10-14_verify_M4_default.txt](2026-10-14_verify_M4_default.txt) | Moved after verified copy | Untracked | 5748 | `85aab2b9b108ac6c5760e53fc77f752c7a394e289182f2730a589356b8d2d4a7` | `85aab2b9b108ac6c5760e53fc77f752c7a394e289182f2730a589356b8d2d4a7` |
| `app_program/subsystem2/verify_M4_optimized.log` | [2026-10-14_verify_M4_optimized.txt](2026-10-14_verify_M4_optimized.txt) | Moved after verified copy | Untracked | 5770 | `dd789847f1d9c00f18cf2652804c670595a3405819725ed0710f2f7efcafeb44` | `dd789847f1d9c00f18cf2652804c670595a3405819725ed0710f2f7efcafeb44` |
| `app_program/subsystem2/verify_M5_default.log` | [2026-10-14_verify_M5_default.txt](2026-10-14_verify_M5_default.txt) | Moved after verified copy | Untracked | 6111 | `fd886c732f36d459f319cccb84c940683bd1df1b1aaf0651f6909a9123957a53` | `fd886c732f36d459f319cccb84c940683bd1df1b1aaf0651f6909a9123957a53` |
| `app_program/subsystem2/verify_M5_optimized.log` | [2026-10-14_verify_M5_optimized.txt](2026-10-14_verify_M5_optimized.txt) | Moved after verified copy | Untracked | 6133 | `fc95da1e03ff8d611452f59a7227a91fc366eced92d595e0122c21b104635e7b` | `fc95da1e03ff8d611452f59a7227a91fc366eced92d595e0122c21b104635e7b` |
| `app_program/subsystem2/verify_M6_default.log` | [2026-10-14_verify_M6_default.txt](2026-10-14_verify_M6_default.txt) | Moved after verified copy | Untracked | 6579 | `54ff1c7f649d9a717a7dd1d9459042e19f47f126632e660133b8eaa8430bcdf4` | `54ff1c7f649d9a717a7dd1d9459042e19f47f126632e660133b8eaa8430bcdf4` |
| `app_program/subsystem2/verify_M6_optimized.log` | [2026-10-14_verify_M6_optimized.txt](2026-10-14_verify_M6_optimized.txt) | Moved after verified copy | Untracked | 6601 | `10b47758c0308eadbd3e18e5390e7514b3a86dc8b785269ce649e8e333d94211` | `10b47758c0308eadbd3e18e5390e7514b3a86dc8b785269ce649e8e333d94211` |
| `app_program/subsystem2/verify_M7_arm.log` | [2026-10-14_verify_M7_arm.txt](2026-10-14_verify_M7_arm.txt) | Moved after verified copy | Untracked | 352392 | `c113b4d003c756d0e4d9fe59ed4da5678d52e68c0fb3e10cbcf883c818fcd28c` | `c113b4d003c756d0e4d9fe59ed4da5678d52e68c0fb3e10cbcf883c818fcd28c` |
| `app_program/subsystem2/verify_M7_default.log` | [2026-10-14_verify_M7_default.txt](2026-10-14_verify_M7_default.txt) | Moved after verified copy | Untracked | 6891 | `26087b957851f9930e083f5242103ecc460997b4cec0f98b0af3ab3c51dcc0ac` | `26087b957851f9930e083f5242103ecc460997b4cec0f98b0af3ab3c51dcc0ac` |
| `app_program/subsystem2/verify_M7_optimized.log` | [2026-10-14_verify_M7_optimized.txt](2026-10-14_verify_M7_optimized.txt) | Moved after verified copy | Untracked | 6913 | `d6285a70f95fa7302c6693e19f04f9a6e75a7c49b3fa157e0d117611ce19ba4c` | `d6285a70f95fa7302c6693e19f04f9a6e75a7c49b3fa157e0d117611ce19ba4c` |
| `app_program/subsystem2/verify_strict_arm.log` | [2026-10-14_verify_strict_arm.txt](2026-10-14_verify_strict_arm.txt) | Moved after verified copy | Untracked | 376624 | `4ae409a134cdf9091815ea6b90ce5d01cad86f03049b46e44edb4c954d2080f3` | `4ae409a134cdf9091815ea6b90ce5d01cad86f03049b46e44edb4c954d2080f3` |
| `app_program/subsystem2/verify_final_arm.log` | [2026-10-14_verify_final_arm.txt](2026-10-14_verify_final_arm.txt) | Moved after verified copy | Untracked | 377688 | `2d5b855d0e4a62c4257096880aba30c07d837ea5f19d3f3529c8558fab0c11b0` | `2d5b855d0e4a62c4257096880aba30c07d837ea5f19d3f3529c8558fab0c11b0` |
| `app_program/subsystem2/verify_final_default.log` | [2026-10-14_verify_final_default.txt](2026-10-14_verify_final_default.txt) | Moved after verified copy | Untracked | 7174 | `531d53ede3c68e9d7d4f22388ff50f60928b6ac893073790e3e9f4198c45f89d` | `531d53ede3c68e9d7d4f22388ff50f60928b6ac893073790e3e9f4198c45f89d` |
| `app_program/subsystem2/verify_final_optimized.log` | [2026-10-14_verify_final_optimized.txt](2026-10-14_verify_final_optimized.txt) | Moved after verified copy | Untracked | 7196 | `de478b08a16f6e937ca7161c8c39276c4b2f5a21adddc85ebbcfc53095a5cd3a` | `de478b08a16f6e937ca7161c8c39276c4b2f5a21adddc85ebbcfc53095a5cd3a` |
| `app_program/subsystem2/verify_final_stack.log` | [2026-10-14_verify_final_stack.txt](2026-10-14_verify_final_stack.txt) | Moved after verified copy | Untracked | 20650 | `a12cce681960e810f0c92417accab1fed1aafa118d9c98826036c471446b1041` | `a12cce681960e810f0c92417accab1fed1aafa118d9c98826036c471446b1041` |
| `app_program/subsystem2/m2_make_recipes.txt` | [2026-10-14_m2_make_recipes.txt](2026-10-14_m2_make_recipes.txt) | Copy; source retained | Tracked | 18460 | `99bad487374dc95052f72497c86af38ba450c77472489a230c1ccfc979e9f0c9` | `99bad487374dc95052f72497c86af38ba450c77472489a230c1ccfc979e9f0c9` |

The two M2 host outputs are byte-identical; both are retained as separately named
profile records. No deduplication was performed. Their profile attribution is
from the historical motor narrative; the raw M2 host logs do not contain
distinct compiler commands or process exit markers.

A redundant PowerShell Split-Path/LiteralPath/Parent check produced non-terminating
parameter-set errors during relocation. The earlier Python preflight had already
validated absolute source/destination parents and all copy hashes. Corrected
.NET parent checks and a full before/after repository hash comparison then
confirmed exactly the authorised moves, no additional removals and no code
changes. Archive integrity did not depend on that redundant failed expression.

## Available outcomes and honest provenance

The [official test record](../TEST_LOG.md) defines all stable IDs S2-U01–U97,
retains the original placeholder row and imports 1,010 actual historical host
outcomes plus all 288 saved compiler attempts. Compiler failures are preserved.
These are imported outcomes, not fresh cleanup tests.

| Evidence family | Available result | Revision information and limits |
|---|---|---|
| M2 host | 54 PASS lines per saved profile, S2-U03–U56 | Reported uncommitted working tree; exact base commit/full snapshot not recorded alongside raw logs |
| M3 host | 59 PASS lines per profile | Continuous-development handoff reports base 9e40a56d… plus dirty intermediate changes; exact intermediate snapshot unavailable |
| M4 host | 61 PASS lines per profile | Same preliminary working-tree qualification |
| M5 host | 68 PASS lines per profile | Same preliminary working-tree qualification |
| M6 host | 80 PASS lines per profile | Same preliminary working-tree qualification |
| M7 initial host | 88 PASS lines per profile | Same preliminary working-tree qualification |
| Final host | 95 PASS lines per profile, S2-U03–U97 | Base 9e40a56d… plus dirty changes; archived ownership manifest records final source hashes and a later token-preserving line wrap |
| S2-U01/U02 | No archived execution evidence; NOT RUN | Native demonstration checks are excluded from the 95-group host runner |
| M2 ARM archive | 60 recorded compiler EXITs: 58 zero, 2 nonzero | Preliminary object compilation; failure diagnostics retained; exact fix commits unavailable |
| M7, strict and final ARM archives | Each contains 76 successful compiler EXITs, 19 files across TEST=0/1 and SMP=0/1 | Preliminary compilation only; exact intermediate states are not fully archived |
| Final stack archive | Individual compiler frame estimates | Not measured target stack margin, interrupt usage or full call-chain bounds |
| Native TEST=1, hardware, GPIO IRQ and physical motion | No execution evidence; NOT RUN / NOT HARDWARE-VALIDATED | No successful complete firmware link or physical validation is implied |

All uncommitted outcomes are **PRELIMINARY**, including the final 95-group host
runs. They are not formal commit-verified evidence. The commit-before-testing
requirement cannot be satisfied retrospectively. The development documents
report 2026-10-14; exact run clock times and original operator attribution are
unrecorded. Do not infer a tested revision from the commit later storing a log,
an archive filename, a filesystem timestamp or the cleanup HEAD.

The earlier foundation narrative reports base
`2886fa8a0118185674f59964ab832d7c4e3ceabb` plus dirty changes. No separate original
foundation execution log exists in this approved archive; no standalone result
was fabricated. Prior verification labels could overwrite log files. All
currently available originals are preserved, but this cannot establish that
every past failed execution was saved.

## Path dependencies inspected

- verify_motion.py reads only the original source-folder m2_make_recipes.txt,
  which remains present and byte-identical.
- The script does NOT read the relocated logs. It still WRITES future logs,
  exe/o/d/su files and TEMP/TMP to the source folder, and repeated labels can
  overwrite those future source-folder records. The script was not modified.
- External scratch output and immutable evidence naming remain recommended
  future tooling work requiring separate approval. Environment TEMP/TMP alone
  cannot override this script's hardcoded values.
- Current handoff/motor-document archive references were updated. The foundation
  verification narrative is unchanged because it has no relocated-log link.
- Raw logs and the historical ownership report intentionally retain original
  paths, temporary filenames and compiler-input names. Use the inventory above
  to resolve old names. These are not live missing source files.
- Historical reproduction commands in old narratives describe old revisions;
  no compiler was invoked to reproduce them in this cleanup.
- No Makefile change, source restructuring or test relocation was needed.

## Preserved production/test/support identity

All 38 C/H files and the verification script below match their initial cleanup
SHA256. This preserves the 31 production files, 7 test C/H files and one support
script (8 testing/support files). All three technical documents remain in
subsystem2; only the two necessary documentation-link edits above differ.

| Source/support repository path | Before and after SHA256 | Result |
|---|---|---|
| `app_program/subsystem2/drv_encoder.c` | `f92daf16ac1943b39fd29e22cd6f63c35a3043e875472b7e1aaeecd420e2ef3e` | Unchanged |
| `app_program/subsystem2/drv_encoder.h` | `caa9f52de95f0710c6ca86f1e6779171d3dbadc40e447b3c08d6b9b0750ba235` | Unchanged |
| `app_program/subsystem2/drv_motor.c` | `1fd72f7b37fe5c807c529f96b34dee8a0d5aacc28e20fbc4149854cfa3e9a81d` | Unchanged |
| `app_program/subsystem2/drv_motor.h` | `7d0bad1946f3400e03cdc135b7c0d1b4bf63320e0f91a462fc6fe2fff8673e41` | Unchanged |
| `app_program/subsystem2/drv_rp2040.c` | `db14ea3c0428c60447775341f7aa2960d3b63be80f70d262331aa4bd1595eb40` | Unchanged |
| `app_program/subsystem2/drv_rp2040.h` | `ddb95b10f1b9af36a045e7a106992bbe876013d6bb61a1717dfde139ea1cdef8` | Unchanged |
| `app_program/subsystem2/enc_capture.c` | `586878820dee0bf82abc03b716be40727a9acee310ed4e09cbb9b9ba3bbccf87` | Unchanged |
| `app_program/subsystem2/enc_capture.h` | `3ee5ff131bd9aa8a545c67f92da8483a83ce878946fb618c132c99cda68e6670` | Unchanged |
| `app_program/subsystem2/motion_api.h` | `07ec7ef7c0613c5542dacadf43a1ec4fb4afe1cb3baf89ecc5a6a313c9cee484` | Unchanged |
| `app_program/subsystem2/motion_bus.c` | `19ad1ef597c806193ca5afff70ab0f8652fc4a47b89d4bad4782df608a282768` | Unchanged |
| `app_program/subsystem2/motion_bus.h` | `8491e984eb398be793d53eedab8bc6b154d36ed5f0d3dd5684b115448b5fa7eb` | Unchanged |
| `app_program/subsystem2/motion_config.c` | `6882d38a75107067a888c85649e858f3a962ca94fd24c5bf72d9965e1d7ec74f` | Unchanged |
| `app_program/subsystem2/motion_config.h` | `c877655d6f6264df6b2b4a4ed95dfa5e04f7f97020126cce751d6b277ad49731` | Unchanged |
| `app_program/subsystem2/motion_estimator.c` | `28f0524c23711b8abe5d31d8fe43ae4d84838fea812cfd10ff735c22fc998baf` | Unchanged |
| `app_program/subsystem2/motion_estimator.h` | `5dd22b79a02706f673331d1d522a051a5b7acd61e0a589146aa25aec4dbe78f9` | Unchanged |
| `app_program/subsystem2/motion_foundation_test.c` | `36056556a7324cc503892c8637833ce679c362ed7e4f98e62b0f8ca6abcc89d4` | Unchanged |
| `app_program/subsystem2/motion_foundation_test.h` | `2d0de8e7f9eaae6d6b58f08e100c5568c186b4ae53a774f18fb383165738002c` | Unchanged |
| `app_program/subsystem2/motion_pid.c` | `688c0edf5ad9ae5a68aee9aef8692dd026ed055e6a8d3c3ad0cb7c8fcd67d6e1` | Unchanged |
| `app_program/subsystem2/motion_pid.h` | `5f46af0f138580d6728d798067173f321dd33c4f7c00771b8051aba9fb4bfeb0` | Unchanged |
| `app_program/subsystem2/motion_quad.c` | `b6127f5392d1f7420988228c54457633375374ce86e613e9b3540e050a71613d` | Unchanged |
| `app_program/subsystem2/motion_quad.h` | `0f5e1757229802b7b5edd20a8b254a9b963b686a0a2b43cde8b553465b3d6972` | Unchanged |
| `app_program/subsystem2/motion_run.c` | `cd9239b966e8cbf60106a34cad5f349b2cf19a10fe6d6855ba6178be89b9d0c9` | Unchanged |
| `app_program/subsystem2/motion_run.h` | `5c04fb91b51d318acd9c992636b61194eeca3be8f251442ad88c110a88a41f42` | Unchanged |
| `app_program/subsystem2/motion_types.h` | `0a76753d2e16197188c2874ffdfea3fde8cb3a94053d93371a3bbefc9a649625` | Unchanged |
| `app_program/subsystem2/move_plan.c` | `727d78a41c066b142bfd92efeda157271e6da9aaa83289e514f7fd2d32ae7aaa` | Unchanged |
| `app_program/subsystem2/move_plan.h` | `35b0496acb1def9c86497ff59bbdbdf8138cbcd0c2c2847ec98ee95f274160c9` | Unchanged |
| `app_program/subsystem2/subsystem2.c` | `26b17efd7c8819c7830b643a588b85e18421e61fa6c39e4902714e7d3b907228` | Unchanged |
| `app_program/subsystem2/subsystem2.h` | `32659be92095df9e5f1d2ec280df1a25905c6b906b1c3a8faeafb728a64d0d2f` | Unchanged |
| `app_program/subsystem2/subsystem2_test.c` | `5d593125e20e92a25cc1f095a20dcab05e84ec6d60f57c616114f5195ba291d0` | Unchanged |
| `app_program/subsystem2/task_motion.c` | `6c4418c6f577c8fa04a782575fbcfc501e8c4f13e555c2253100c4ad929f58bc` | Unchanged |
| `app_program/subsystem2/task_motion.h` | `0776fa9b415fc07ed00b9258cbcbabcfd29e2df0256e7b3a3a9511ad61751bf0` | Unchanged |
| `app_program/subsystem2/task_producer.c` | `b8f1cabf4ffacbf95136e247a843fe60a06a2345503421b7283486b0b8006ba4` | Unchanged |
| `app_program/subsystem2/test_drive.c` | `7d1b563d2b1bc643b353f75238c94f4b5cc6a738e0128f3b206ff9c8f168c646` | Unchanged |
| `app_program/subsystem2/test_drive.h` | `9a83c11b8a5f11bec84f69555dd8582c2ab0a47799c4ec953ee53cdfc8f07256` | Unchanged |
| `app_program/subsystem2/test_motor.c` | `3895dcdedf0fafba016854404ea981550f43360bd27e8dca09c457ffc45bcdcb` | Unchanged |
| `app_program/subsystem2/test_motor.h` | `d1cb984f9fa1bf0d1c1418f2f0bc3e4c1094ff5a40af69e19f181a6438af6f04` | Unchanged |
| `app_program/subsystem2/verify_motion.py` | `8f16c303beacfe2e2115f6ff03de2c2a90290c911534c86672786d0632f4cc61` | Unchanged |
| `app_program/subsystem2/wheel_loop.c` | `7697b40c05693952a7015b7f2757926556b60e3fa95372c8425bf9285df9e141` | Unchanged |
| `app_program/subsystem2/wheel_loop.h` | `2e79492ffba21a7151f894658a30e10e5a19bf89c6fd58584e8de55046c45d34` | Unchanged |

## Cleanup verification and final structure

A whole-repository content comparison, excluding .git, found exactly:

- 20 removed SOURCE paths, each represented by its byte-identical archive.
- 21 added .txt archives: 20 moved logs and one recipe copy.
- This one new manifest.
- 3 modified text documents: the official TEST_LOG and two necessary link updates.
- No unexpected added, modified or removed file; all 719 files outside the two
  authorised Buddy 2 folders remain byte-identical.
- No original history/evidence content lost. The previous TEST_LOG placeholder
  row and S2-U01/U02 definitions remain preserved.
- Source folder: 43 files = 31 production C/H + 8 testing/support +
  3 technical documents + 1 original recipe.
- Testing folder: 24 files = TEST_LOG.md + preserved logs/.gitkeep +
  21 evidence .txt files + this manifest.

Content checks compare every imported per-ID host outcome and compiler attempt
against the original archives, confirm 97 unique ordered definitions and verify
local documentation links (excluding the explicitly labelled inherited
placeholder path). No runtime test is claimed by these content checks.

The source-folder modifications already present at task start are unrelated
development changes and were not edited by this cleanup. Future staging must
include the intended source files and archive relocations explicitly; no Git
publication is authorised by this record.


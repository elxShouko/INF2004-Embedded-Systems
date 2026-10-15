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

# Subsystem 5 — test log

**Subsystem:** Buddy 5 · **Code:** `app_program/subsystem5/`

See `testing/README.md` for how to run tests and record results.

## Test cases

Define a test here before running it. Test IDs never change.

| ID | What is tested | Requirement / interface | Method | Steps / how | Expected result |
|---|---|---|---|---|---|
| S5-U01 | Test entry runs in a task context | µT-Kernel test hook | auto | `subsystem5_test.c` | `[TEST] S5-U01 PASS` |
| S5-U02 | Generate bounded coarse and fine scan angles | Development scan geometry | auto | `host/test_obstacle_logic.c` | Both edge-clamped and centred angle lists are correct |
| S5-U03 | Build obstacle position, width and side range profile | Scan profile calculations | auto | `host/test_obstacle_logic.c` | Nearest point, projected position, width estimate and side ranges match the fixture |
| S5-U04 | Identify a left-side range candidate without authorizing motion | Avoidance planning | auto | `host/test_obstacle_logic.c` | Candidate metadata may be set, but no actions are returned and the plan is not executable |
| S5-U05 | Check reverse candidacy at the development threshold | Avoidance planning | auto | `host/test_obstacle_logic.c` | Reverse is advisory metadata only; no reverse action is emitted |
| S5-U06 | Handle missing side-range candidates without claiming safety | Avoidance planning | auto | `host/test_obstacle_logic.c` | No bypass candidate is reported; no action plan is returned |
| S5-U07 | Reject malformed scan input | Scan input validation | auto | `host/test_obstacle_logic.c` | Invalid input is reported and the profile is cleared |
| S5-U08 | Identify a right-side range candidate without authorizing motion | Avoidance planning | auto | `host/test_obstacle_logic.c` | Candidate metadata may be set, but no actions are returned and the plan is not executable |
| S5-U09 | Reject too few valid fine-scan readings | Scan input validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_INSUFFICIENT_DATA`; no profile is produced |
| S5-U10 | Reject an uninitialized profile when building a plan | Avoidance planning | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_INVALID_ARGUMENT`; plan remains empty |
| S5-U11 | Profile fine scans around off-axis coarse directions | Scan profile calculations | auto | `host/test_obstacle_logic.c` | Scans centered at 30, 60, 120, and 150 degrees produce the expected projected coordinates |
| S5-U12 | Reject a cancelled scan | Scan context validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_SCAN_CANCELLED`; profile is cleared |
| S5-U13 | Reject acquisition or servo-positioning failure | Scan context validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_ACQUISITION_FAILED`; profile is cleared |
| S5-U14 | Reject incomplete scans and scans while the vehicle is moving | Scan context validation | auto | `host/test_obstacle_logic.c` | Incomplete or non-stationary scan is rejected; profile is cleared |
| S5-U15 | Enforce configured maximum scan duration | Scan context validation | auto | `host/test_obstacle_logic.c` | Over-duration scan is rejected; profile is cleared |
| S5-U16 | Reject non-monotonic sample timestamps | Scan context validation | auto | `host/test_obstacle_logic.c` | Invalid sample chronology is rejected; profile is cleared |
| S5-U17 | Reject adjacent range jumps over the supplied limit | Scan consistency validation | auto | `host/test_obstacle_logic.c` | Inconsistent scan is rejected; profile is cleared |
| S5-U18 | Enforce configured maximum scan age | Scan context validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_STALE_SCAN`; profile is cleared |
| S5-U19 | Reject samples captured after scan completion | Scan context validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_STALE_SCAN`; profile is cleared |
| S5-U20 | Reject trigger, echo, and servo acquisition fault statuses | Acquisition fault handling | auto | `host/test_obstacle_logic.c` | Each non-OK acquisition status rejects the scan and clears the profile |
| S5-U21 | Reject missing scan-validation configuration | Scan context validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_INVALID_SCAN`; profile is cleared |
| S5-U22 | Reject an out-of-range measurement | Scan input validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_INVALID_SCAN`; profile is cleared |
| S5-U23 | Accept valid timestamps across millisecond-counter wrap | Scan context validation | auto | `host/test_obstacle_logic.c` | Valid wrapped scan duration, sample ordering, and scan age are accepted |

*Method:* **auto** = an automated check prints a result; **manual** = a person
observes the result (LED, scope, meter, phone app…). S5-U02–S5-U11 are host
checks; S5-U12–S5-U23 are also host-only context-validation checks. None
exercise the RTOS, sensors, servo, vehicle, or hardware safety. The acquisition
fault cases test rejection of supplied status values; they do not prove a
physical driver can detect those faults. S5-U01 remains defined but has no
recorded result; the integrated `TEST=1` image and on-target test have not been
run.

## Results

Add a new row for every run. Never edit or delete old rows.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S5-U02 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U03 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U04 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U05 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U06 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U07 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U08 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U09 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U10 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic.txt` | Host logic only; uncommitted Buddy 5 changes |
| 2026-10-15 | S5-U02 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U03 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U04 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U05 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Checks 9, 10, and 11 cm against the development threshold |
| 2026-10-15 | S5-U06 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U07 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U08 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U09 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U10 | `a508994 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_merge_check.txt` | Re-run after threshold-boundary coverage; host logic only |
| 2026-10-15 | S5-U02 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U03 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U04 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U05 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U06 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U07 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U08 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U09 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U10 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Off-axis projection fix; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U11 | `a405c5a + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_logic_final_review.txt` | Confirms projection for 30, 60, 120, and 150 degree scan centers |
| 2026-10-15 | S5-U02 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Strict C99 host run; updated fail-closed planner |
| 2026-10-15 | S5-U03 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Strict C99 host run; updated fail-closed planner |
| 2026-10-15 | S5-U04 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Confirms range candidate is not an executable route |
| 2026-10-15 | S5-U05 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Confirms reverse remains advisory metadata |
| 2026-10-15 | S5-U06 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Missing candidates are not treated as proven unsafe |
| 2026-10-15 | S5-U07 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Malformed scan rejected |
| 2026-10-15 | S5-U08 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Confirms right range candidate is non-executable |
| 2026-10-15 | S5-U09 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Insufficient fine readings rejected |
| 2026-10-15 | S5-U10 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Uninitialized profile rejected |
| 2026-10-15 | S5-U11 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Off-axis profile projection checks |
| 2026-10-15 | S5-U12 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Cancelled scan rejected |
| 2026-10-15 | S5-U13 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Echo timeout and unsettled servo rejected |
| 2026-10-15 | S5-U14 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Incomplete or moving-vehicle scan rejected |
| 2026-10-15 | S5-U15 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Configured maximum duration enforced |
| 2026-10-15 | S5-U16 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Non-monotonic timestamps rejected |
| 2026-10-15 | S5-U17 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Excessive adjacent range change rejected |
| 2026-10-15 | S5-U18 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Maximum scan age enforced |
| 2026-10-15 | S5-U19 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Sample after completion rejected |
| 2026-10-15 | S5-U20 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Trigger, echo, and servo fault values rejected |
| 2026-10-15 | S5-U21 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Missing scan-validation limit rejected |
| 2026-10-15 | S5-U22 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Out-of-range distance rejected |
| 2026-10-15 | S5-U23 | `4240895 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_host_fail_closed_review.txt` | Valid scan crosses uint32 millisecond-counter wrap |

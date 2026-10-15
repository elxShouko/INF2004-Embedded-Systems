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
| S5-U04 | Select a left-side plan from the range heuristic | Avoidance plan generation | auto | `host/test_obstacle_logic.c` | Stop, turn, bypass, return-turn and line-recovery actions are ordered |
| S5-U05 | Check reverse planning at the development threshold | Avoidance plan generation | auto | `host/test_obstacle_logic.c` | Reverse is included at or below the development threshold and omitted just above it |
| S5-U06 | Reject a profile with no route meeting the range heuristic | Avoidance planning | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_NO_SAFE_ROUTE`; no action plan is returned |
| S5-U07 | Reject malformed scan input | Scan input validation | auto | `host/test_obstacle_logic.c` | Invalid input is reported and the profile is cleared |
| S5-U08 | Select a right-side plan from the range heuristic | Avoidance plan generation | auto | `host/test_obstacle_logic.c` | Right and return turns match the selected side |
| S5-U09 | Reject too few valid fine-scan readings | Scan input validation | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_INSUFFICIENT_DATA`; no profile is produced |
| S5-U10 | Reject an uninitialized profile when building a plan | Avoidance planning | auto | `host/test_obstacle_logic.c` | `S5_LOGIC_INVALID_ARGUMENT`; plan remains empty |
| S5-U11 | Profile fine scans around off-axis coarse directions | Scan profile calculations | auto | `host/test_obstacle_logic.c` | Scans centered at 30, 60, 120, and 150 degrees produce the expected projected coordinates |

*Method:* **auto** = an automated check prints a result; **manual** = a person
observes the result (LED, scope, meter, phone app…). S5-U02–S5-U11 are host
checks and do not exercise the RTOS, sensors, servo, vehicle, or hardware safety.
S5-U01 remains defined but has no recorded result; the integrated `TEST=1`
image and on-target test have not been run.

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

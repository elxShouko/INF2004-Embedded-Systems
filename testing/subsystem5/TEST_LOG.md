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
| S5-U24 | Reject missing runtime timeout configuration | Runtime policy configuration | auto | `host/test_obstacle_runtime.c` | Runtime initialization rejects zero timeout values |
| S5-U25 | Fail closed on invalid forward data | Forward protection | auto | `host/test_obstacle_runtime.c` | Stop and fault requests are emitted |
| S5-U26 | Require distinct repeated proximity readings | Forward protection | auto | `host/test_obstacle_runtime.c` | Duplicate timestamps do not confirm an obstacle; thresholded distinct readings request a stop |
| S5-U27 | Require stopped acknowledgement before full scan | Stop/scan sequencing | auto | `host/test_obstacle_runtime.c` | Scan request is emitted only after stopped status |
| S5-U28 | Fail closed on stop acknowledgement timeout | Stop/scan sequencing | auto | `host/test_obstacle_runtime.c` | Stop and fault requests are emitted |
| S5-U29 | Fail closed on scan cancellation | Scan cancellation | auto | `host/test_obstacle_runtime.c` | Stop and fault requests are emitted; no route is produced |
| S5-U30 | Fail closed if vehicle moves during scan | Scan safety | auto | `host/test_obstacle_runtime.c` | Stop and fault requests are emitted |
| S5-U31 | Enforce the configured full-scan timeout | Scan timeout | auto | `host/test_obstacle_runtime.c` | Stop and fault requests are emitted |
| S5-U32 | Reject acquisition failures | Scan fault handling | auto | `host/test_obstacle_runtime.c` | Echo acquisition failure does not produce a profile or route |
| S5-U33 | Refuse bypass without verified clearance and copy measurements | Clearance/status | auto | `host/test_obstacle_runtime.c` | A valid profile with unverified clearance emits no bypass request; bounded sample/profile data are copied to status |
| S5-U34 | Copy scan and fault information into status | Runtime status snapshot | auto | `host/test_obstacle_runtime.c` | Snapshot contains completion, validity, and fault status |
| S5-U35 | Keep stop/fault requests asserted after a latched fault | Runtime fault handling | auto | `host/test_obstacle_runtime.c` | Repeated status steps keep requesting stop and fault handling |
| S5-U36 | Fail closed on invalid runtime input | Runtime fault handling | auto | `host/test_obstacle_runtime.c` | Invalid input emits stop/fault requests and latches an invalid-input fault |
| S5-U37 | Fail closed on stale forward data | Forward protection | auto | `host/test_obstacle_runtime.c` | Stale data emits stop/fault requests |
| S5-U38 | Reject backwards runtime time | Runtime time validation | auto | `host/test_obstacle_runtime.c` | Backwards time emits stop/fault requests |
| S5-U39 | Accept runtime clock wrap | Runtime time validation | auto | `host/test_obstacle_runtime.c` | A valid uint32 millisecond wrap does not fault |
| S5-U40 | Bound scan startup/acquisition and report busy state | Scan lifecycle/status | auto | `host/test_obstacle_runtime.c` | Active scan is reported busy; failure to complete by timeout fails closed |
| S5-U41 | Request line recovery after bypass acknowledgement | Recovery sequencing | auto | `host/test_obstacle_runtime.c` | Bypass completion emits a line-recovery request |
| S5-U42 | Fail closed on bypass acknowledgement timeout | Recovery timeout | auto | `host/test_obstacle_runtime.c` | Stop/fault requests are emitted |
| S5-U43 | Resume monitoring on line-found acknowledgement | Recovery sequencing | auto | `host/test_obstacle_runtime.c` | `LINE_FOUND` returns runtime policy to monitoring |
| S5-U44 | Fail closed on line-reacquisition timeout | Recovery timeout | auto | `host/test_obstacle_runtime.c` | Stop/fault requests are emitted |
| S5-U45 | Keep forward obstacle protection active during bypass | Forward protection | auto | `host/test_obstacle_runtime.c` | A renewed close forward reading faults and requests stop |
| S5-U46 | Keep requesting stop until stopped acknowledgement | Stop/scan sequencing | auto | `host/test_obstacle_runtime.c` | Stop remains requested in `WAIT_STOP` and alongside scan request on transition to `SCANNING` |
| S5-U47 | Keep requesting stop throughout stationary scanning | Scan safety | auto | `host/test_obstacle_runtime.c` | An active scan continues to request stop without raising a fault |

*Method:* **auto** = an automated check prints a result; **manual** = a person
observes the result (LED, scope, meter, phone app…). S5-U02–S5-U11 are host
checks; S5-U12–S5-U23 are also host-only context-validation checks. None
exercise the RTOS, sensors, servo, vehicle, or hardware safety. The acquisition
fault cases test rejection of supplied status values; they do not prove a
physical driver can detect those faults. S5-U01 remains defined but has no
recorded result; S5-U24–S5-U45 test a pure, single-owner-task policy module and
status-copy shape only. The integrated `TEST=1` image and on-target test have
not been run. S5-U41–S5-U45 seed the policy state directly to cover branches
that remain unreachable through normal planning because clearance is unverified;
they do not authorize or demonstrate an executable bypass. S5-U46–S5-U47
check that stop requests persist during stop acknowledgement and scanning;
they still do not prove that an external controller consumes those requests.

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
| 2026-10-15 | S5-U24 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Strict C99 runtime policy check; missing timeout rejected |
| 2026-10-15 | S5-U25 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Invalid forward sample requests stop/fault |
| 2026-10-15 | S5-U26 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Distinct readings required before stop |
| 2026-10-15 | S5-U27 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Full scan request follows stopped acknowledgement |
| 2026-10-15 | S5-U28 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Stop timeout requests stop/fault |
| 2026-10-15 | S5-U29 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Cancelled scan requests stop/fault |
| 2026-10-15 | S5-U30 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Movement during stationary scan rejected |
| 2026-10-15 | S5-U31 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Configured scan timeout enforced |
| 2026-10-15 | S5-U32 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Echo acquisition error blocks profile/route |
| 2026-10-15 | S5-U33 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Status copies measurements; clearance-unverified plan emits no bypass |
| 2026-10-15 | S5-U34 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Servo acquisition fault appears in copied status |
| 2026-10-15 | S5-U35 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Latched fault keeps stop/fault requested |
| 2026-10-15 | S5-U36 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Null input fails closed |
| 2026-10-15 | S5-U37 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Stale forward sample fails closed |
| 2026-10-15 | S5-U38 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Backwards runtime time fails closed |
| 2026-10-15 | S5-U39 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Valid uint32 timestamp wrap is accepted |
| 2026-10-15 | S5-U40 | `f3e77ff + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_policy.txt` | Active scan busy status reported; missing completion times out |
| 2026-10-15 | S5-U02 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U03 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U04 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U05 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U06 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U07 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U08 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U09 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U10 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U11 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U12 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U13 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U14 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U15 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U16 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U17 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U18 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U19 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U20 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U21 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U22 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U23 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 scan-logic host suite |
| 2026-10-15 | S5-U24 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U25 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U26 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U27 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U28 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U29 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U30 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U31 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U32 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U33 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U34 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U35 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U36 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U37 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U38 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U39 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U40 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U41 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Branch-level bypass completion to line-recovery transition |
| 2026-10-15 | S5-U42 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Branch-level bypass acknowledgement timeout |
| 2026-10-15 | S5-U43 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Branch-level LINE_FOUND recovery transition |
| 2026-10-15 | S5-U44 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Branch-level line-reacquisition timeout |
| 2026-10-15 | S5-U45 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_prepush.txt` | Branch-level forward obstacle during bypass |
| 2026-10-15 | S5-U46 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_hold.txt` | Stop request persists until stopped acknowledgement |
| 2026-10-15 | S5-U47 | `afbbbd7 + WT` | GCC 14.2 | PASS | `logs/2026-10-15_runtime_hold.txt` | Stop request persists during stationary scanning |

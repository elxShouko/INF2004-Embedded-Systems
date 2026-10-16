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
| S5-U89 | Reject a stopped status sampled before stop request | Acknowledgement freshness | auto | `host/test_obstacle_runtime.c` | Stale stopped status does not start a scan; stop remains requested |
| S5-U90 | Reject stale stopped status during a scan | Scan safety | auto | `host/test_obstacle_runtime.c` | Stale stop status faults and requests stop |
| S5-U91 | Reject bypass completion from before the bypass request | Acknowledgement freshness | auto | `host/test_obstacle_runtime.c` | Pre-request completion cannot begin line recovery |
| S5-U92 | Reject LINE_FOUND from before the recovery request | Acknowledgement freshness | auto | `host/test_obstacle_runtime.c` | Pre-request line evidence cannot resume monitoring |
| S5-U93 | Enforce bypass acknowledgement deadline | Recovery timeout | auto | `host/test_obstacle_runtime.c` | Completion at the deadline faults instead of being accepted |
| S5-U94 | Require a configured acknowledgement freshness limit | Runtime policy configuration | auto | `host/test_obstacle_runtime.c` | Zero acknowledgement freshness is rejected |
| S5-U95 | Report completion separately from scan validity | Scan lifecycle/status | auto | `host/test_obstacle_runtime.c` | A malformed completed scan is marked complete, invalid, and faulted |
| S5-U96 | Clear old scan status on a new obstacle sequence | Runtime status lifecycle | auto | `host/test_obstacle_runtime.c` | New request clears previous timestamps, validity, and acquisition fault |
| S5-U97 | Revoke forward validity when forward sensing faults | Forward status validity | auto | `host/test_obstacle_runtime.c` | A stale forward sample faults and status reports the last forward value invalid |
| S5-U98 | Retry scan request until scan owner acknowledges busy | Scan request delivery | auto | `host/test_obstacle_runtime.c` | Repeated request is asserted until scan-busy is reported |
| S5-U99 | Retry bypass request until completion | Bypass request delivery | auto | `host/test_obstacle_runtime.c` | Request persists while waiting; completion stops retries and requests recovery |
| S5-U100 | Retry line recovery until LINE_FOUND | Recovery request delivery | auto | `host/test_obstacle_runtime.c` | Request persists while waiting and clears after line-found acknowledgement |

*Method:* **auto** = an automated check prints a result; **manual** = a person
observes the result (LED, scope, meter, phone app…). S5-U02–S5-U11 are host
checks; S5-U12–S5-U23 are also host-only context-validation checks. None
exercise the RTOS, sensors, servo, vehicle, or hardware safety. The acquisition
fault cases test rejection of supplied status values; they do not prove a
physical driver can detect those faults. S5-U01 remains defined but has no
recorded result. S5-U24–S5-U100 test a pure, single-owner-task policy module
and status-copy shape only. The integrated `TEST=1` image and on-target test
have not been run. S5-U41–S5-U45 and S5-U91–S5-U93 seed the policy
state directly to cover branches that remain unreachable through normal
planning because clearance is unverified; they do not authorize or demonstrate
an executable bypass. S5-U46–S5-U47 check that stop requests persist during
stop acknowledgement and scanning; they still do not prove that an external
controller consumes those requests. S5-U89–S5-U94 validate timestamped,
post-request acknowledgements and their freshness/deadline limits. They do not
provide request IDs or prove correlation with any unagreed external interface.
S5-U95 verifies completion remains distinct from validity when scan data is
malformed. S5-U96 verifies scan status does not leak across obstacle sequences.
S5-U97 verifies forward data is not advertised as valid after a forward-input
fault. S5-U98–S5-U100 verify requests are held for retry until their matching
provider acknowledgement. They do not specify the transport or request-ID
contract needed for cross-task integration.

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


## Software completion verification (2026-10-16)

Preserved S5-U02..U47 and added the cases below to the existing runtime suite
plus one focused MMIO-port fake. Host files remain under testing/subsystem5.
S5-U01 physical verification remains unverified. Pure policy bypass branch tests
do not establish an executable geometric avoidance planner.

Commands: python testing/integration/verify_integration.py --label
completion_release_host; same runner --arm --label completion_race_verified_arm.
Default and optimized (-O2 -fshort-enums), strict-warning/GCC analyzer profiles:
71 unique S5-U02..U72 PASS each. RTOS cases execute the real owner task in
bounded subprocesses with typed resource fakes; adapter cases execute the real
drv_s5.c with fake MMIO/IRQ hooks. No real GPIO activation. Full firmware links
and ARM object results are recorded in ../integration/TEST_LOG.md.

Evidence: ../integration/logs/software_completion_evidence.zip, final
completion_verified entries and manifest. Tested base HEAD
5c08a5c76de39ffe18d9720b40d505a592508230 plus exact hashed uncommitted sources.

| New ID | Required PASS behavior |
|---|---|
| S5-U48 | Explicit wiring/calibration/pin/timing validation; no defaults |
| S5-U49 | Repeated acquisition open preserves original instance |
| S5-U50 | ISR capture after clock read defers, then preserves actual range/time |
| S5-U51 | Moving scan request is deferred |
| S5-U52 | Full coarse/fine scan, >60 ms inter-trigger spacing |
| S5-U53 | Close is idempotent and clears validity |
| S5-U54 | Pending echo faults exactly at deadline |
| S5-U55 | Motion during scan aborts acquisition |
| S5-U56 | Regressing acquisition clock faults |
| S5-U57 | Cancellation reports ABORTED rather than echo timeout |
| S5-U58 | Typed RTOS partial-init cleanup preserves IDs/error and allows retry |
| S5-U59 | Unconfigured startup never opens GPIO |
| S5-U60 | Actual owner publishes original source time and valid forward range |
| S5-U61 | Timeout closes once and publishes latched fault state/cause |
| S5-U62 | Failed start retries; repeated start/configuration rejects |
| S5-U63 | Stale status invalidates; POL contention preserves caller buffer |
| S5-U64 | Actual MMIO adapter reserves ECHO and owned outputs only |
| S5-U65 | PWM7 endpoints preserve motor PWM4/5 sentinels |
| S5-U66 | Actual ISR callback captures rise/fall width and timestamp |
| S5-U67 | Adapter close is idempotent |
| S5-U68 | Existing GPIO owner prevents open |
| S5-U69 | Reservation failure never activates outputs |
| S5-U70 | Partial GPIO failure releases owned subscription |
| S5-U71 | Enable failure rolls back outputs/reservation |
| S5-U72 | Duplicate echo edge marks acquisition error |

Target scan/capture timing, voltage safety, PWM endpoints, range accuracy,
fault latency and stack headroom remain unverified. Range-only planning still
returns CLEARANCE_UNVERIFIED and zero actions. Safe bypass and moving recovery
software remain incomplete pending a validated geometry/coverage/error model.
Authoritative guide: ../../docs/buddy5/OBSTACLE_SYSTEM.md.

Late self-review: U50 now simulates an ISR completing after the cycle clock
read. The production engine defers consumption without restamping or losing
the sample. Reintroducing the old rejection makes U50 fail (confirmed mutation).
Final reruns use completion_race_verified_* evidence; earlier attempts retained.


## Configured bypass and recovery verification (2026-10-16)

Extends the existing logic/runtime/port suites; all earlier S5-U02..U72 retained.
No new test files or runners. Strict C99/GCC analyzer default and
-O2 -fshort-enums profiles: S5-U02..U88, 87 unique groups PASS/profile.
Hardware fakes exercise real planner, scan engine, owner and MMIO port.
The full live-owner U86 runs scanning -> held profile -> exact-ID acknowledgement
-> monitoring within one uninterrupted task invocation; it checks wrong tokens,
idempotence and clearing the old completed scan. No task restart is simulated.

| ID | Definition / expected behavior |
|---|---|
| S5-U73 | Left route distances in um and action order match explicit fixture geometry |
| S5-U74 | Right turns and free-space corridors mirror the left route |
| S5-U75 | Missing verification clears output and inhibits execution |
| S5-U76 | Clearance from another scan frame rejects |
| S5-U77 | Evidence older than completed scan rejects |
| S5-U78 | Future evidence rejects |
| S5-U79 | Validity must cover the entire route deadline |
| S5-U80 | One-um swept-path undercoverage rejects |
| S5-U81 | Free-space claim intersecting the bounded obstacle rejects |
| S5-U82 | Missing stopping calibration and uint64 time overflow reject |
| S5-U83 | Real valid scan holds for controller without inventing clearance |
| S5-U84 | Recovery handshake rearms monitoring and invalidates old scan |
| S5-U85 | Held scan still faults on unavailable forward sensing |
| S5-U86 | Real task scan/hold/recovery API sequence and typed object checks |
| S5-U87 | Free space without original-line identity cannot authorize recovery |
| S5-U88 | Legacy positive-right offset converts to positive-left route frame |

Commands/results: ../integration/logs/software_completion_evidence.zip,
route_verified_host_{default,optimized}.log, route_verified_arm_arm.log.
Base HEAD 5c08a5c76de39ffe18d9720b40d505a592508230 plus uncommitted source;
exact current source/test hashes are archived. These are software tests, not
physical certificates. No field dimensions/gains/margins were invented.

## Preserved incoming main results (historical IDs)

These rows retain their original revision-specific IDs. See the merge mapping
below for current IDs; historical PASS results are not new merged-run results.

| Date | Test ID | Commit | Tester | Result | Log / evidence | Notes |
|---|---|---|---|---|---|---|
| 2026-10-15 | S5-U02 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U03 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U04 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U05 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U06 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U07 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U08 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U09 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U10 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U11 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U12 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U13 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U14 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U15 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U16 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U17 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U18 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U19 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U20 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U21 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U22 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U23 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U24 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U25 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U26 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U27 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U28 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U29 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U30 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U31 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U32 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U33 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U34 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U35 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U36 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U37 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U38 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U39 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U40 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U41 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Branch-level bypass-to-line-recovery test |
| 2026-10-15 | S5-U42 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Branch-level bypass timeout |
| 2026-10-15 | S5-U43 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Branch-level LINE_FOUND handling |
| 2026-10-15 | S5-U44 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Branch-level line timeout |
| 2026-10-15 | S5-U45 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Branch-level forward-obstacle handling |
| 2026-10-15 | S5-U46 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Stop request persists until stopped acknowledgement |
| 2026-10-15 | S5-U47 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Stop request persists during scanning |
| 2026-10-15 | S5-U48 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Pre-stop stopped sample rejected |
| 2026-10-15 | S5-U49 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Stale stopped status faults during scan |
| 2026-10-15 | S5-U50 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Pre-request bypass completion rejected |
| 2026-10-15 | S5-U51 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Pre-request LINE_FOUND rejected |
| 2026-10-15 | S5-U52 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Bypass acknowledgement deadline enforced |
| 2026-10-15 | S5-U53 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_ack_freshness.txt` | Missing acknowledgement-age limit rejected |
| 2026-10-15 | S5-U02 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U03 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U04 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U05 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U06 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U07 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U08 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U09 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U10 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U11 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U12 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U13 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U14 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U15 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U16 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U17 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U18 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U19 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U20 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U21 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U22 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U23 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 host logic suite |
| 2026-10-15 | S5-U24 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U25 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U26 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U27 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U28 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U29 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U30 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U31 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U32 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U33 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U34 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U35 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U36 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U37 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U38 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U39 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U40 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Strict C99 runtime suite and GCC analyzer |
| 2026-10-15 | S5-U41 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Direct-state bypass-to-recovery policy test |
| 2026-10-15 | S5-U42 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Direct-state bypass timeout policy test |
| 2026-10-15 | S5-U43 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Direct-state LINE_FOUND policy test |
| 2026-10-15 | S5-U44 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Direct-state line timeout policy test |
| 2026-10-15 | S5-U45 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Direct-state forward-obstacle policy test |
| 2026-10-15 | S5-U46 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Persistent stop request policy test |
| 2026-10-15 | S5-U47 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Persistent stop request during scan |
| 2026-10-15 | S5-U48 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Pre-stop acknowledgement rejected |
| 2026-10-15 | S5-U49 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Stale stopped status fails closed |
| 2026-10-15 | S5-U50 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Pre-request bypass completion rejected |
| 2026-10-15 | S5-U51 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Pre-request line-found event rejected |
| 2026-10-15 | S5-U52 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Bypass deadline enforced |
| 2026-10-15 | S5-U53 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Acknowledgement freshness required |
| 2026-10-15 | S5-U54 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_final_host.txt` | Malformed completed scan remains complete and invalid |
| 2026-10-15 | S5-U02 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U03 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U04 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U05 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U06 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U07 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U08 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U09 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U10 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U11 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U12 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U13 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U14 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U15 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U16 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U17 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U18 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U19 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U20 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U21 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U22 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U23 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final strict C99 host logic run |
| 2026-10-15 | S5-U24 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U25 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U26 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U27 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U28 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U29 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U30 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U31 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U32 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U33 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U34 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U35 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U36 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U37 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U38 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U39 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U40 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Final runtime suite; strict C99 and GCC analyzer |
| 2026-10-15 | S5-U41 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Direct-state bypass-to-recovery policy test |
| 2026-10-15 | S5-U42 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Direct-state bypass timeout policy test |
| 2026-10-15 | S5-U43 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Direct-state LINE_FOUND policy test |
| 2026-10-15 | S5-U44 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Direct-state line timeout policy test |
| 2026-10-15 | S5-U45 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Direct-state forward obstacle policy test |
| 2026-10-15 | S5-U46 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Persistent stop request policy test |
| 2026-10-15 | S5-U47 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Persistent stop request during scan |
| 2026-10-15 | S5-U48 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Pre-stop acknowledgement rejected |
| 2026-10-15 | S5-U49 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Stale stopped status fails closed |
| 2026-10-15 | S5-U50 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Pre-request bypass completion rejected |
| 2026-10-15 | S5-U51 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Pre-request line-found event rejected |
| 2026-10-15 | S5-U52 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Bypass deadline enforced |
| 2026-10-15 | S5-U53 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Acknowledgement freshness required |
| 2026-10-15 | S5-U54 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Malformed completed scan remains complete and invalid |
| 2026-10-15 | S5-U55 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-15_status_refresh.txt` | Previous scan status cleared on new sequence |
| 2026-10-16 | S5-U02 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U03 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U04 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U05 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U06 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U07 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U08 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U09 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U10 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U11 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U12 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U13 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U14 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U15 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U16 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U17 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U18 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U19 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U20 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U21 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U22 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U23 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Final strict C99 host logic run |
| 2026-10-16 | S5-U24 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U25 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U26 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U27 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U28 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U29 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U30 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U31 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U32 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U33 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U34 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U35 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U36 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U37 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U38 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U39 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U40 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Runtime host suite; GCC analyzer |
| 2026-10-16 | S5-U41 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Direct-state recovery policy test |
| 2026-10-16 | S5-U42 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Direct-state bypass-timeout policy test |
| 2026-10-16 | S5-U43 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Direct-state LINE_FOUND policy test |
| 2026-10-16 | S5-U44 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Direct-state line-timeout policy test |
| 2026-10-16 | S5-U45 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Direct-state forward-obstacle policy test |
| 2026-10-16 | S5-U46 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Persistent stop-request policy test |
| 2026-10-16 | S5-U47 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Stop remains asserted throughout scanning |
| 2026-10-16 | S5-U48 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Pre-stop stopped sample rejected |
| 2026-10-16 | S5-U49 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Stale stopped status fails closed |
| 2026-10-16 | S5-U50 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Pre-request bypass completion rejected |
| 2026-10-16 | S5-U51 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Pre-request line-found event rejected |
| 2026-10-16 | S5-U52 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Bypass acknowledgement deadline enforced |
| 2026-10-16 | S5-U53 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Acknowledgement freshness required |
| 2026-10-16 | S5-U54 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Malformed complete scan reported invalid |
| 2026-10-16 | S5-U55 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Old scan status cleared on new sequence |
| 2026-10-16 | S5-U56 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Forward validity revoked after stale-data fault |
| 2026-10-16 | S5-U57 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Scan request held until scan-busy acknowledgement |
| 2026-10-16 | S5-U58 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Bypass request held until completion acknowledgement |
| 2026-10-16 | S5-U59 | `5c08a5c + WT` | GCC 14.2 | PASS | `logs/2026-10-16_final.txt` | Recovery request held until LINE_FOUND acknowledgement |


## Merge reconciliation (2026-10-16)

Both branches added different tests under S5-U48..U59. Integration IDs
S5-U48..U88 remain unchanged. Incoming main acknowledgement regressions now
use S5-U89..U100 (old incoming ID + 41). Historical logs/results retain their
original bytes and revision-specific IDs; they are not rewritten or relabelled
as new test evidence. Both complete test implementations are retained.

| Incoming historical ID | Current merged ID |
|---|---|
| S5-U48 | S5-U89 |
| S5-U49 | S5-U90 |
| S5-U50 | S5-U91 |
| S5-U51 | S5-U92 |
| S5-U52 | S5-U93 |
| S5-U53 | S5-U94 |
| S5-U54 | S5-U95 |
| S5-U55 | S5-U96 |
| S5-U56 | S5-U97 |
| S5-U57 | S5-U98 |
| S5-U58 | S5-U99 |
| S5-U59 | S5-U100 |

Merge-resolution verification: default and optimized strict/analyzer host
profiles PASS: 79 integration IDs and 99 unique S5-U02..U100 IDs each, with
eight integration mutations detected/profile. All 44 strict ARM objects PASS.
No full firmware link or physical test was repeated in this merge-resolution
pass. Evidence and current source hashes: ../integration/logs/
software_completion_evidence.zip, merge_resolution_* entries.

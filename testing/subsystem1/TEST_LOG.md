# Subsystem 1 test log

Run the PC tests from any directory: `testing/subsystem1/host/run_host_tests.sh`
(needs bash and gcc). The exit status is the verdict: 0 only if every build and test passed.
Each run saves its commands, revision metadata and raw output to
`testing/subsystem1/logs/host_run_<revision>.txt`.

## Test cases

| ID | What is tested | Requirement | Where it runs |
|---|---|---|---|
| S1-U01..U05 | JSON fields, worst-case size, command parser, backoff table, heartbeat | FR17, NFR10, FR18, FR19, FR20 | PC + Pico (`TEST=1`) |
| S1-U06, U07 | telemetry buffer order, full buffer returns `E_TMOUT` without blocking | IF-06 | PC + Pico |
| S1-U08..U10 | E-STOP flag and queue, rejects unknown text, E-STOP evicts when full | FR18, NFR7, IF-07, IF-08 | PC + Pico |
| S1-U11 | CONNECTED flushes stale frames, raises link-changed; DISCONNECTED clears | IF-08 | PC + Pico |
| S1-U12 | simulator makes about 10 frames per second | FR16 | Pico only (simulator off on the PC) |
| S1-U13a..e | entering BACKOFF from connected clears the state, counts one disconnect and raises link-changed once; repeats while down do not re-notify | feedback 3 | PC + Pico |
| S1-U14a..d | writer on one core, reader on the other, 20000 updates: every snapshot coherent, no lost update, both states observed | feedback 2 | PC (real threads) + Pico (two cores) |
| S1-U15a..c | simulator enable switch: 5000 changes from another core, silent when off, running when on | feedback 2 | Pico only |
| S1-U16a..d | EMERGENCY_STOP with the diagnostic mutex held by another core and the command queue full: latch and pulse published while the hook is blocked; `get_stats` times out, `is_connected` and `estop_latched` never block; stop still reaches the queue | safety review 1 | PC (real threads); skipped without threads |
| S1-U17..U19 | latch set only by EMERGENCY_STOP; RESUME, START_RUN, PAUSE, link events and a queue drain do not clear it; only `s1_net_estop_clear()` does | safety review 2 | PC + Pico |
| S1-U20a,b | a failed flag post is retried; a persistent failure is counted and the queued command still delivers the stop | safety review 1 | PC only (fault injection) |
| S1-U21a..d | truncated message and unknown id give E_SYS with zeroed output and a count; kernel E_TMOUT unchanged; the queue works afterwards | safety review 2 | PC + Pico |
| S1-U22a..c | a command older than 1 s is dropped and counted; an old EMERGENCY_STOP is not | safety review 2 | PC + Pico |
| W1 | first join request refused, second accepted: retry after 1 s, then normal operation | feedback 1, FR15 | PC (scripted driver, real `cyw43_utk_task`) |
| W2 | every join request refused: gaps 1, 2, 4, 8, 8, 8 s; the task never waits more than 2 ms | feedback 1, FR19 | PC |
| W3 | link up, then lost: rejoin after 1 s again (not the old 4 s step) | feedback 1, FR19 | PC |
| W4 | radio or interface bring-up failure is permanent: no join, not ready, network not polled | feedback 1 | PC |
| W5 | interface lost during a retry: retries stop, reported not ready | feedback 1 | PC |
| H1 | no `cyw43_` / lwIP / MQTT call in `app_program/subsystem1` (only the hook registration) | TEAM.md section 3 | PC (grep) |

## Results

| Revision | Test | Result | Evidence |
|---|---|---|---|
| 9b34dae | `run_host_tests.sh`: codec 33, backoff 15, MQTT 33, Wi-Fi task 17 checks; emulated-kernel application tests 24 passed, 0 failed (real threads) and again in no-thread mode; H1 | PASS, runner exit 0 | `logs/host_run_9b34dae.txt` |
| 9b34dae | Windows build of the emulated-kernel harness (MinGW cross compiler, run under wine) | PASS (24 passed) | in `logs/host_run_9b34dae.txt`, section 5 |
| 9b34dae | Negative controls: status mutex removed (S1-U14b fails, 46 incoherent snapshots), BACKOFF without notification (S1-U13b fails), Wi-Fi task before the fix (13 of 17 checks fail), runner exit status when a check count is missed (exit 1) | all fail as intended | `logs/negative_controls_9b34dae.txt` |
| 04f9c2a (code identical) | `make`, `make NET=lwip MQTT=1`, `make NET=lwip MQTT=1 TEST=1`, `make TEST=1`, SDK 2.2.0, arm-none-eabi-gcc 13.2.1 | build OK, 0 warnings in each | `logs/make_all_configs_9b34dae.txt` |
| 04f9c2a | BARR-C:2018 kit (format, lexical, tidy) on `s1_net.c/.h`, `s1_sim.c`, `subsystem1_test.c` | clean, except 3 `readability-isolate-declaration` hits that come from the template's `TEST_INIT` macro in the test file | run by hand, not saved |
| (this change) | `run_host_tests.sh` after the safety-path change: 43 checks pass with real threads, 40 in no-thread mode; four `make` profiles build with 0 warnings against current main | PASS | `logs/host_run_*.txt` |
| (this change) | Negative controls: emergency published after the lock (S1-U16a,b,d fail), size check removed (S1-U21a fails), stale drop removed (S1-U22a..c fail) | all fail as intended | run by hand |
| pending | **Pico, `make TEST=1`**: S1-U12, S1-U14, S1-U15 on two real cores | not run | save the console output as `logs/pico_test_<date>.txt` |
| pending | **Pico, `make NET=lwip MQTT=1`**: first join refused (for example wrong SSID, then correct), then AP power-cycle | not run | save serial output |
| pending | Hardware: 5-minute delivery (NFR8), CPU during an outage (NFR9) | not run | |

## Limits of this evidence

* The PC tests prove the logic and, with real threads, the locking discipline. They do not prove
  behaviour on the RP2040's two cores; S1-U14 and S1-U15 must be run on the Pico.
* The Wi-Fi tests use a scripted driver. They prove the decisions in `cyw43_utk_task`, not what the
  CYW43 firmware does on a real access point.
* Emergency latency (NFR7, 50 ms) is not measured. The host tests show the ORDER of operations and that no lock or
  full queue can delay the publish; the time itself needs a scope or GPIO timestamp on the Pico.


## Software completion verification (2026-10-16)

Tested HEAD 5c08a5c76de39ffe18d9720b40d505a592508230 plus the final uncommitted
software-completion working tree. This is source-hash-bound preliminary evidence,
not a claim that a committed final revision was tested.

Command: S1_NO_LOG=1 bash testing/subsystem1/host/run_host_tests.sh (MSYS2,
GCC 14.2). Final result: 43 threaded application checks, 40 codec checks,
15 backoff checks, 33 MQTT checks and 17 Wi-Fi checks PASS. The optional Windows
cross-build compiled; wine was unavailable, so that executable was not run.
The MSYS shell could not locate Git: raw output says revision unknown; the
external archive manifest records the actual repository revision/source hashes.

Root causes corrected: zero-valued TWF_ANDW was tested as a nonzero flag in
the fake, completing multi-bit AND waits prematurely; U14's busy reader could
consume its finite budget before the writer completed. Corrected AND/OR
semantics now have a direct regression, and periodic 1 ms test waits preserve
all coherence, overlap and totals assertions. Host copied-message capacity
is now 128 bytes, accommodating the enlarged production frame.

Seven additional codec checks cover signed INT64 endpoints, UINT64 source-time
precision, missing/null fields, invalid generation/validity, age bounds,
capacity failure and legacy behavior. User-approved production v2 keys and
dashboard/FR17 mapping: ../../docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md.
Full output: ../integration/logs/software_completion_evidence.zip,
entry testing/integration/logs/completion_verified_s1.log. Original failed
attempts are preserved in that archive. No physical networking or SMP claim.


## Configured bypass and recovery verification (2026-10-16)

Existing host script PASS: 43 threaded checks, 41 codec, 15 backoff, 33 MQTT,
17 Wi-Fi checks. The added codec check enumerates all validity masks with
full negative widths and longest fields: maximum v2 payload 239 bytes,
240 including NUL. The largest mask is 59 (unknown line). Legacy format/tests
remain unchanged. Optional Windows cross-build compiled but not executed
without wine. Evidence: ../integration/logs/software_completion_evidence.zip,
route_final_s1.log. No dashboard sources were found; the external contract is
../../docs/buddy1/TELEMETRY_COMMAND_CONTRACT.md.


## Barcode mapping - 2026-10-26 (definitions before execution)

| ID | Method | Expected result |
|---|---|---|
| S1-U23 | Host/on-target copied command hook | Complete four-letter MAP accepted, malformed/duplicate/partial payload rejected; timestamp and copied letters survive queue |


### 2026-10-26 mapping results

All runs: HEAD `6d225bfa818dd31d8c5ff5f1351c434326c470d3` plus uncommitted
barcode changes on `buddy3-barcode`. Native JSON reports include source hashes.

| Date | Test | Outcome | Evidence | Notes |
|---|---|---|---|---|
| 2026-10-26 | Host runner | BLOCKED by environment | logs/2026-10-26_mapping_1.txt | Sandbox prevented MSYS initialization; native runner used instead. |
| 2026-10-26 | S1-U23 build | FAIL, link | logs/2026-10-26_mapping_native_2.json | New fixture used tk_get_tim; changed to existing operating-clock API tk_get_otm. |
| 2026-10-26 | Existing codec/backoff/MQTT/Wi-Fi suites and S1-U23 | PASS, host | logs/2026-10-26_mapping_native_3.json; logs/2026-10-26_mapping_final.json | Actual mapping hook/queue passed; 44 emulated-kernel checks, zero failures, pthread tests enabled. Radio and real SMP not exercised. |
| 2026-10-26 | Same suites after formatting-only test initializer wrap | PASS, host | logs/2026-10-26_mapping_final2.json | No production behaviour changed after the earlier final run. |

Command: `python testing/subsystem1/host/verify_windows.py --log testing/subsystem1/logs/2026-10-26_mapping_final2.json`.

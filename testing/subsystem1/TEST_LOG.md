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
| pending | **Pico, `make TEST=1`**: S1-U12, S1-U14, S1-U15 on two real cores | not run | save the console output as `logs/pico_test_<date>.txt` |
| pending | **Pico, `make NET=lwip MQTT=1`**: first join refused (for example wrong SSID, then correct), then AP power-cycle | not run | save serial output |
| pending | Hardware: 5-minute delivery (NFR8), CPU during an outage (NFR9) | not run | |

## Limits of this evidence

* The PC tests prove the logic and, with real threads, the locking discipline. They do not prove
  behaviour on the RP2040's two cores; S1-U14 and S1-U15 must be run on the Pico.
* The Wi-Fi tests use a scripted driver. They prove the decisions in `cyw43_utk_task`, not what the
  CYW43 firmware does on a real access point.

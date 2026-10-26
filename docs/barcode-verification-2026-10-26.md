# Barcode change verification - 2026-10-26

> Historical pre-merge evidence. The subsequent overlap cleanup on main is
> recorded in the appended subsystem/integration test logs. The current
> transport contract is [barcode-demo.md](barcode-demo.md).

Working branch: `buddy3-barcode`. Base HEAD:
`6d225bfa818dd31d8c5ff5f1351c434326c470d3`, with uncommitted changes.
No staging, commit, push, dependency download, flashing or hardware access.
The new requirement supersedes the older immediate-barcode-turn contract.

## Relevant changes

- S1 validates a complete runtime four-letter map and copies it through the
  existing command queue. The shared controller converts it into an S3 update.
  Full S3 queue admission is retried within the original command age limit.
- S3 starts with no movement assignments. A complete accepted map replaces
  every previous assignment. Invalid/pending-instruction updates leave it alone.
- A decoded mapped letter/action remains in owner RAM while line following
  continues. An actionable junction event begins the existing motor-owner
  stop/manoeuvre/completion/recovery flow. The letter is consumed once.
- The controller ignores barcode observations as finite-movement triggers and
  preserves actionable junction/fault events during a bounded queue drain.
- Existing A-Z/reverse decoding, motor actuation ownership, kernel internals,
  pins, control gains and calibration values are unchanged.

The full reviewable diff is the uncommitted working-tree diff. Source hashes,
commands and raw outputs accompany the test records below. The S1 on-target
test initializer received a formatting-only wrap after the first final build;
the later native run and MQTT TEST build use that exact formatting.

## Host checks

Commands from the repository root, using the bundled Python executable
`C:/Users/linco/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`:

```text
python testing/subsystem3/verify_production_timing.py --gcc C:/msys64/ucrt64/bin/gcc.exe --log testing/subsystem3/logs/2026-10-26_barcode_requirements_final.json
python testing/subsystem1/host/verify_windows.py --log testing/subsystem1/logs/2026-10-26_mapping_final2.json
python testing/integration/verify_integration.py --label barcode_20261026_final2
```

| Check | Result |
|---|---|
| S3 existing IR/decoder/junction/owner/production/feedback/packed/safety suites | PASS |
| S3-U38, S3-U39: empty map, replacement, four actions, both directions, wait and repeat | PASS |
| S1 existing codec/backoff/MQTT/Wi-Fi and emulated-kernel suites, S1-U23 | PASS; 44 kernel-harness checks, zero failures, threads enabled |
| Shared controller/runtime, INT-82..85 plus existing groups | PASS; 82 groups in each of default and optimized/fshort-enums profiles |
| Existing intentional safety/telemetry/route mutations | All eight detected in both profiles |
| Existing TEST=1 startup isolation and S5 policy/acquisition/live/MMIO regressions | PASS, host |
| `git diff --check` | PASS |

The S3 mechanical Barr subset passes; it is not full Barr-C certification.
The shared runner exposes pre-existing S4 conversion warnings without making
them fatal, and uses a representable invalid value in its packed IMU fixture.
Buddy 4 source was not modified. Failed intermediate runs and fixture fixes
remain recorded in the subsystem/integration test logs.

Historical behaviour-equivalence scripts are not acceptance checks for an
intentional change in the mission contract.

## GNU Make firmware builds

Used the professor's `build_make/` application with installed GNU Make through
MSYS Bash on Windows. WSL tools were not available through the checked command.
Pico SDK: `C:/Users/linco/.pico-sdk/sdk/2.3.1`.
Arm compiler: installed `arm-none-eabi-gcc` under
`C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin`.
The compiler installation name is independent of the selected SDK source.

The first attempt used the old SDK source and failed on missing `pico.h`.
Switching SDKs exposed a stale TinyUSB object; a complete `make -B -j8` rebuild
fixed the mismatch without changing supplied sources or Make rules.

Final commands from `build_make/`, with the SDK/compiler paths set as above:

```sh
make -j8
make TEST=1 -j8
make NET=lwip MQTT=1 "INCPATH=-I../include -I../config -I../kernel/knlinc -I../build/barcode-build-config" -j8
make NET=lwip MQTT=1 TEST=1 "INCPATH=-I../include -I../config -I../kernel/knlinc -I../build/barcode-build-config" -j8
```

The network builds use an ignored build-only copy of
`config/wifi_secrets.example.h` at `build/barcode-build-config/wifi_secrets.h`.
These are placeholder network values, not a configured demo image. No actual
credentials were created or used. The normal build defaults remain
`SMP=1 CONSOLE=usb_cdc WIFI=cyw43 NET=none MQTT=0 TEST=0 CRASH_REPORT=0`.

Build results and output hashes are recorded in
`testing/integration/logs/barcode_20261026_build_metadata.json`; raw logs are
the `barcode_20261026_final_*_build.log` files in that directory.

## Remaining physical evidence

Compilation and host simulation do not prove physical scanning accuracy,
cross-junction identity, turn placement, stopping distance or SMP timing.
There is still no default measured mission setup caller in production startup.
Commission the actual hardware and local network, enable calibrated junction
detection, supply fresh odometry, and validate all four movements at the
actual cross-junction. The current detector confirms after side-white
clearance, so sensor position and manoeuvre origin require measurement.

No MQTT application-level mapping acknowledgement topic was added. Verify the
S3 accepted reply/applied map revision before a demo; broker delivery alone
does not prove application acceptance. See [demo contract](barcode-demo.md).

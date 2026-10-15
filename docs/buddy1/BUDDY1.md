# Buddy 1 - Wi-Fi, MQTT, commands, telemetry, connection recovery

## What micro T-Kernel means here (NFR11)
The template runs the firmware on micro T-Kernel (uT-Kernel 3.0), a small real-time OS.
You do not write a `main()` loop. You create **tasks** and let the kernel schedule them, and
tasks talk through kernel objects instead of shared variables:

| Kernel object | Calls used | Buddy 1 use |
|---|---|---|
| Task | `tk_cre_tsk`, `tk_sta_tsk` | simulator task, Wi-Fi service task (template) |
| Cyclic handler | `tk_cre_cyc` | 100 ms telemetry tick (simulator) |
| Message buffer | `tk_cre_mbf`, `tk_snd_mbf`, `tk_rcv_mbf` | IF-06 telemetry, IF-07 commands |
| Mutex | `tk_cre_mtx`, `tk_loc_mtx`, `tk_unl_mtx` | one coherent status snapshot, simulator switch |
| Event flag | `tk_cre_flg`, `tk_set_flg`, `tk_wai_flg` | IF-08 command / E-STOP / link flags |
| Time | `tk_get_otm`, `tk_dly_tsk` | timestamps, uptime |

Only the Wi-Fi service task touches the radio and lwIP. Other Buddies never call MQTT directly.

## Contract for the other Buddies
* Send ONE aggregated `s1_codec_frame_t` every 100 ms with `s1_net_telemetry_send()` (never blocks).
* Receive commands: `s1_net_events_wait()` then `s1_net_command_receive()`.
  `S1_NET_FLG_ESTOP` is set the moment EMERGENCY_STOP arrives.
* When the real sensors feed the frame, set `S1_SIMULATE_TELEMETRY` to 0.
* The real units, signed encoder counts, line encoding, validity, command consumer and
  emergency-stop path are NOT agreed yet: see
  [TELEMETRY_COMMAND_CONTRACT.md](TELEMETRY_COMMAND_CONTRACT.md) (a proposal for the team).

## Shared status and SMP
The Wi-Fi task (writer) and application tasks (readers) can run on different cores, so the
status block is read and written only under one brief mutex (`s1_net_get_stats()` returns a
single coherent copy). Nothing waits and no network call is made while it is held. `volatile`
or aligned loads are not used as a synchronisation method.

## Files
app_program/subsystem1: s1_codec (JSON), s1_net (buffers/flags), s1_sim, subsystem1*.
lib/liblwip/.../mqtt_utk.c + net_backoff.h (non-blocking MQTT, backoff 1,2,4,8 s),
lib/libwifi/.../cyw43_utk.c (rejoin with backoff). Host tests: testing/subsystem1/host.

## Topics
robot/telemetry (100 ms JSON), robot/status (1 s heartbeat, last will OFFLINE), robot/cmd (in).

## Build / flash (WSL)
    cp config/wifi_secrets.example.h config/wifi_secrets.h   # edit SSID, password, broker IP
    cd build_make && export PICO_SDK_PATH=~/pico-sdk && make NET=lwip MQTT=1 -j8
Hold BOOTSEL, drop the .uf2 on the Pico. Broker IP changes with every hotspot session.

## Status
See `testing/subsystem1/TEST_LOG.md` for the exact results, the revision they were run on and
the raw logs. Summary:
* PC: codec, backoff, MQTT client, Wi-Fi task rejoin logic (scripted driver) and the application
  tests on an emulated kernel pass. `testing/subsystem1/host/run_host_tests.sh` exits non-zero
  on any failure and saves its output.
* All four `make` configurations build with 0 warnings; BARR-C format and clang-tidy are clean
  on the Buddy 1 files (the template's `TEST_INIT` macro still trips one tidy check in the test file).
* NOT yet run on hardware with this build: S1-U14/U15 (two-core status test), first-join recovery,
  the 5-minute delivery run (NFR8) and the outage CPU load (NFR9).
Known template-name exceptions: `tid_led`/`cyc_led` (NOLINT), and `subsystem1`/`subsystem1_test`
share their first 8 characters (template-owned file names).

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
| Event flag | `tk_cre_flg`, `tk_set_flg`, `tk_wai_flg` | IF-08 command / E-STOP / link flags |
| Time | `tk_get_otm`, `tk_dly_tsk` | timestamps, uptime |

Only the Wi-Fi service task touches the radio and lwIP. Other Buddies never call MQTT directly.

## Contract for the other Buddies
* Send ONE aggregated `s1_codec_frame_t` every 100 ms with `s1_net_telemetry_send()` (never blocks).
* Receive commands: `s1_net_events_wait()` then `s1_net_command_receive()`.
  `S1_NET_FLG_ESTOP` is set the moment EMERGENCY_STOP arrives.
* When the real sensors feed the frame, set `S1_SIMULATE_TELEMETRY` to 0.

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
Host and emulated-kernel tests pass; all four make configurations build; BARR-C format and
clang-tidy are clean on the Buddy 1 files. NOT yet run on hardware with this build.
Known template-name exceptions: `tid_led`/`cyc_led` (NOLINT), and `subsystem1`/`subsystem1_test`
share their first 8 characters (template-owned file names).

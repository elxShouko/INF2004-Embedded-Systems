"""Run existing S1 host suites with native GCC, without a Bash runtime."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[3]
    host = root / 'testing/subsystem1/host'
    app = root / 'app_program/subsystem1'
    gcc = 'C:/msys64/ucrt64/bin/gcc.exe'
    checks = []
    report = {
        'head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
        'status': subprocess.check_output(['git', 'status', '--short'], text=True),
        'scope': 'Host simulation; no real radio, RTOS scheduling or robot',
        'sha256': {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                   for p in list(app.glob('*.c')) + list(app.glob('*.h')) + [Path(__file__)]},
        'checks': checks}

    def run(command):
        result = subprocess.run([str(p) for p in command], cwd=host,
                                capture_output=True, text=True, timeout=60)
        checks.append({'command': [str(p) for p in command],
                       'exit': result.returncode, 'stdout': result.stdout,
                       'stderr': result.stderr})
        print(result.stdout + result.stderr, end='')
        return result.returncode == 0

    scratch = root / 'build/s1-native'
    scratch.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='mapping-', dir=scratch) as temp:
        temp = Path(temp)
        os.environ['TEMP'] = os.environ['TMP'] = str(temp)
        lwip = root / 'lib/liblwip/sysdepend/pico_rp2040'
        wifi = root / 'lib/libwifi/sysdepend/pico_rp2040'
        flags = [gcc, '-std=c99', '-Wall', '-Wextra', '-Wshadow', '-Wundef']
        suites = {
            'codec': ['test_codec_host.c', app / 's1_codec.c'],
            'backoff': ['test_backoff_host.c'],
            'mqtt': ['-Imocks', '-I' + str(lwip), 'test_mqtt_host.c', lwip / 'mqtt_utk.c'],
            'wifi': ['-DTM_NET_LWIP=1', '-DTM_MQTT=1', '-DCNF_SMP=1',
                     '-Imocks_wifi', '-I' + str(wifi), '-I' + str(lwip),
                     'test_wifi_host.c', wifi / 'cyw43_utk.c']}
        for name, sources in suites.items():
            exe = temp / (name + '.exe')
            if run(flags + sources + ['-o', exe]):
                run([exe])
        sync = temp / 'host_sync.o'
        run([gcc, '-std=gnu11', '-Wall', '-Wextra', '-DHOST_THREADS=1',
             '-pthread', '-c', 'host_sync.c', '-o', sync])
        exe = temp / 'target.exe'
        command = [gcc, '-std=gnu11', '-Wall', '-Wextra', '-pthread',
                   '-Wno-int-to-pointer-cast', '-DPROHIBIT_DEF_SIZE_T',
                   '-include', 'stddef.h', '-DAPP_TEST=1', '-D_PICO_RP2040_',
                   '-DS1_SIMULATE_TELEMETRY=0', '-DTM_MQTT=0',
                   '-DS1_TEST_HOST_FAULTS=1', '-DHOST_THREADS=1',
                   '-I' + str(root / 'include'), '-I' + str(root / 'config'),
                   '-I' + str(root / 'app_program'), 'kernel_emulation.c', sync,
                   'target_main.c', app / 's1_net.c', app / 's1_codec.c',
                   app / 's1_sim.c', app / 'subsystem1_test.c', '-o', exe]
        if run(command):
            run([exe])
    report['passed'] = all(check['exit'] == 0 for check in checks)
    args.log.write_text(json.dumps(report, indent=2) + '\n')
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())

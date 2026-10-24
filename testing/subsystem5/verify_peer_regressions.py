"""Reproduce S1/S4 host-script commands on Windows without editing peer files.

S2/S3 and shared integration retain their own existing Python entrypoints.
No network, Make, target access, Git mutation, or generated peer sources.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import runpy
import subprocess

ROOT = Path(__file__).resolve().parents[2]
TOOLS = runpy.run_path(str(ROOT / 'testing/subsystem2/verify_motion.py'))
parser = argparse.ArgumentParser()
parser.add_argument('--label', required=True)
args = parser.parse_args()
if not args.label.replace('_', '').isalnum():
    parser.error('label must be alphanumeric/underscore')
scratch = ROOT / 'testing/subsystem5/build' / args.label
scratch.mkdir(parents=True, exist_ok=False)
env = os.environ.copy()
env['TEMP'] = env['TMP'] = str(scratch)
env['PATH'] = str(Path(TOOLS['HOST_GCC']).parent) + os.pathsep + env['PATH']
checks = []


def run(command):
    command = [str(part) for part in command]
    result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True,
                            text=True, timeout=60)
    checks.append({'command': command, 'exit': result.returncode,
                   'stdout': result.stdout, 'stderr': result.stderr})
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    return result.stdout


def build_run(name, flags, sources, cases=None):
    exe = scratch / (name + '.exe')
    run([TOOLS['HOST_GCC'], *flags, *sources, '-o', exe])
    for case in ([None] if cases is None else cases):
        run([exe] if case is None else [exe, str(case)])


def s1():
    app = ROOT / 'app_program/subsystem1'
    host = ROOT / 'testing/subsystem1/host'
    lwip = ROOT / 'lib/liblwip/sysdepend/pico_rp2040'
    wifi = ROOT / 'lib/libwifi/sysdepend/pico_rp2040'
    flags = ['-std=c99', '-Wall', '-Wextra', '-Wshadow', '-Wundef',
             '-Wswitch-default', '-Wfloat-equal']
    build_run('s1_codec', flags, [host / 'test_codec_host.c', app / 's1_codec.c'])
    build_run('s1_backoff', flags, [host / 'test_backoff_host.c'])
    build_run('s1_mqtt', flags + ['-I' + str(host / 'mocks'), '-I' + str(lwip)],
              [host / 'test_mqtt_host.c', lwip / 'mqtt_utk.c'])
    build_run('s1_wifi', ['-std=gnu11', '-Wall', '-Wextra', '-DTM_NET_LWIP=1',
              '-DTM_MQTT=1', '-DCNF_SMP=1', '-I' + str(host / 'mocks_wifi'),
              '-I' + str(wifi), '-I' + str(lwip)],
              [host / 'test_wifi_host.c', wifi / 'cyw43_utk.c'])
    sync = scratch / 's1_host_sync.o'
    run([TOOLS['HOST_GCC'], '-std=gnu11', '-Wall', '-Wextra', '-DHOST_THREADS=1',
         '-pthread', '-c', host / 'host_sync.c', '-o', sync])
    build_run('s1_threaded', ['-std=gnu11', '-Wall', '-Wextra',
        '-Wno-int-to-pointer-cast', '-DPROHIBIT_DEF_SIZE_T', '-include', 'stddef.h',
        '-DAPP_TEST=1', '-D_PICO_RP2040_', '-DS1_SIMULATE_TELEMETRY=0',
        '-DTM_MQTT=0', '-DS1_TEST_HOST_FAULTS=1', '-DHOST_THREADS=1', '-pthread',
        '-I' + str(ROOT / 'include'), '-I' + str(ROOT / 'config'),
        '-I' + str(ROOT / 'app_program')],
        [host / 'kernel_emulation.c', sync, host / 'target_main.c',
         app / 's1_net.c', app / 's1_codec.c', app / 's1_sim.c',
         app / 'subsystem1_test.c'])


def s4():
    app = ROOT / 'app_program/subsystem4'
    host = ROOT / 'testing/subsystem4/host'
    flags = ['-std=c99', '-Wall', '-Wextra', '-Werror', '-Wconversion',
             '-Wshadow', '-pedantic', '-O2']
    build_run('s4_core', flags,
        [app / n for n in ('imu_core.c', 'imu_guard.c', 'drv_lsm.c')] +
        [host / 'buddy4_cases.c', host / 'host_main.c', '-lm'])
    build_run('s4_runtime', flags + ['-DS4_IMU_ENABLE=1', '-I' + str(host / 'fake')],
        [app / n for n in ('imu_core.c', 'imu_runtime.c', 'imu_guard.c', 'imu_link.c')] +
        [host / 'runtime_fake.c', host / 'runtime_main.c', '-lm'],
        list(range(149, 162)) + list(range(190, 200)))
    build_run('s4_transport', flags + ['-DS4_IMU_ENABLE=1', '-DTM_WIFI_CYW43=1',
                                     '-I' + str(host / 'fake')],
        [app / 'drv_imu.c', app / 'drv_lsm.c', host / 'transport_main.c'], [202, 203])


inputs = [Path(__file__).resolve()]
for number in (1, 4):
    for folder in (ROOT / f'app_program/subsystem{number}',
                   ROOT / f'testing/subsystem{number}/host'):
        inputs += [p for p in folder.rglob('*') if p.suffix in ('.c', '.h', '.sh')]
report = {'head': run(['git', 'rev-parse', 'HEAD']).strip(),
          'status': run(['git', 'status', '--short']),
          'scope': 'Host fakes only; S1 includes actual pthreads',
          'sha256': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                     for p in sorted(inputs)}, 'suites': {}, 'checks': checks}
run([TOOLS['HOST_GCC'], '--version'])
for name, suite in [('S1', s1), ('S4', s4)]:
    try:
        suite()
        report['suites'][name] = 'PASS'
        print(name, 'PASS')
    except (RuntimeError, subprocess.TimeoutExpired) as error:
        report['suites'][name] = 'FAIL: ' + str(error)
        print(name, report['suites'][name])
log = ROOT / 'testing/subsystem5/logs' / (args.label + '.json')
with log.open('x', encoding='utf-8') as file:
    json.dump(report, file, indent=2)
print(log)
raise SystemExit(0 if all(value == 'PASS' for value in report['suites'].values()) else 1)

"""Run acquisition bounds and real-driver MMIO regressions; no RTOS/hardware."""
from pathlib import Path
import argparse, hashlib, json, os, runpy, subprocess

ROOT = Path(__file__).resolve().parents[2]
TOOLS = runpy.run_path(str(ROOT / 'testing/subsystem2/verify_motion.py'))
parser = argparse.ArgumentParser()
parser.add_argument('--label', required=True)
opts = parser.parse_args()
if not opts.label.replace('_', '').isalnum():
    parser.error('label must be alphanumeric/underscore')
scratch = ROOT / 'testing/subsystem5/build' / opts.label
scratch.mkdir(parents=True, exist_ok=False)
os.environ['TEMP'] = os.environ['TMP'] = str(scratch)
base = ROOT / 'app_program/subsystem5'
host = ROOT / 'testing/subsystem5/host'
with (ROOT / 'testing/subsystem5/logs' / (opts.label + '.txt')).open('x', encoding='utf-8') as log:
    def run(args, expected=0):
        args = [str(arg) for arg in args]
        log.write(subprocess.list2cmdline(args) + '\n'); log.flush()
        result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, timeout=60)
        log.write(result.stdout + result.stderr + f'EXIT {result.returncode}\n'); log.flush()
        if result.returncode != expected:
            raise SystemExit(result.stdout + result.stderr + f'Unexpected exit {result.returncode}')
        return result.stdout
    run(['git', 'rev-parse', 'HEAD'])
    run(['git', 'status', '--short'])
    inputs = sorted(base.glob('*.[ch]')) + sorted(host.glob('*.[ch]')) + [Path(__file__).resolve()]
    log.write(json.dumps({str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                         for p in inputs}, indent=2) + '\n')
    run([TOOLS['HOST_GCC'], '--version'])
    for profile, extra in [('default', []), ('optimized', ['-O2', '-fshort-enums'])]:
        flags = [TOOLS['HOST_GCC'], '-std=c99', *TOOLS['WARN'], '-fanalyzer', *extra]
        exe = scratch / (profile + '.exe')
        run(flags + [host / 'test_representation.c', base / 's5_acquire.c',
                     base / 's5_obstacle_logic.c', '-o', exe])
        output = run([exe])
        if output.count(' PASS ') != 6:
            raise SystemExit('Missing acquisition bounds cases')
        portflags = flags + ['-DS5_DRIVER_HOST_TEST', '-D_PICO_RP2040_', '-DCNF_SMP=1',
                             '-I' + str(ROOT / 'include'), '-I' + str(ROOT / 'config'),
                             '-I' + str(host)]
        portexe = scratch / (profile + '_port.exe')
        run(portflags + [host / 'test_s5_port.c', base / 'drv_s5.c', base / 's5_acquire.c',
                         base / 's5_obstacle_logic.c', '-o', portexe])
        output = run([portexe])
        if output.count(' PASS ') != 17:
            raise SystemExit('Missing driver/MMIO cases')
        # Prove the tests detect the specific numerical/ISR regressions.
        acquire = (base / 's5_acquire.c').read_text()
        mutations = [
            ('(width_us > (uint32_t)p_owner->config.echo_us_per_cm *\n'
             '                    S5_MAX_VALID_DISTANCE_CM)', '(false)', 'S5-U101'),
            ('((uint64_t)width > captured_us - (p_owner->deadline_us -\n'
             '                 p_owner->config.echo_timeout_us))', '(false)', 'S5-U105')]
        for index, (old, new, test_id) in enumerate(mutations):
            if acquire.count(old) != 1:
                raise SystemExit('Mutation anchor mismatch')
            mutated = scratch / f'mutated_{index}.c'
            mutated.write_text(acquire.replace(old, new))
            run(flags + ['-I' + str(base), host / 'test_representation.c', mutated,
                         base / 's5_obstacle_logic.c', '-o', exe])
            output = run([exe], expected=1)
            if f'{test_id} FAIL' not in output:
                raise SystemExit('Mutation failed for unexpected reason')
        print(profile + ': 23 checks PASS; 2 acquisition regressions detected')

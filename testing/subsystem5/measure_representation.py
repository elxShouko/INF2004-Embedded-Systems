"""Record identical host/ARM size probes and S5 object/stack costs; no hardware."""
from pathlib import Path
import argparse, hashlib, json, os, runpy, shlex, subprocess

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
logpath = ROOT / 'testing/subsystem5/logs' / (opts.label + '.txt')
source = ROOT / 'testing/subsystem5/host/measure_representation.c'
with logpath.open('x', encoding='utf-8') as log:
    def run(args):
        log.write(subprocess.list2cmdline([str(a) for a in args]) + '\n')
        log.flush()
        result = subprocess.run(args, cwd=ROOT / 'build_make', capture_output=True,
                                text=True, timeout=60)
        log.write(result.stdout + result.stderr + f'EXIT {result.returncode}\n')
        log.flush()
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
        return result.stdout
    run(['git', 'rev-parse', 'HEAD'])
    run(['git', 'status', '--short'])
    inputs = sorted((ROOT / 'app_program/subsystem5').glob('*.[ch]'))
    log.write(json.dumps({str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                         for p in inputs}, indent=2) + '\n')
    for profile, extra in [('default', []), ('optimized', ['-O2', '-fshort-enums'])]:
        log.write('\nHOST ' + profile + '\n')
        exe = scratch / (profile + '.exe')
        flags = [TOOLS['HOST_GCC'], '-std=c99', *TOOLS['WARN'], *extra,
                 '-DVEHICLE_HOST_TEST', '-DMOTION_RTOS_HOST_TEST', '-DMOTION_CONFIG_HOST_TEST',
                 '-include', str(ROOT / 'testing/integration/test_ports.h')]
        run(flags + [str(source), '-o', str(exe)])
        print(profile, run([str(exe)]))
    recipe = next(s for s in (ROOT / 'testing/subsystem2/m2_make_recipes.txt').read_text().splitlines()
                  if s.startswith('arm-none-eabi-gcc '))
    nm = str(Path(TOOLS['ARM_GCC']).with_name('arm-none-eabi-nm.exe'))
    run([TOOLS['ARM_GCC'], '--version'])
    for name in ['measure_representation.c', 's5_live.c', 'drv_s5.c', 's5_acquire.c']:
        src = source if name == source.name else ROOT / 'app_program/subsystem5' / name
        args = shlex.split(recipe)
        args[0] = TOOLS['ARM_GCC']
        for i, arg in enumerate(args):
            if arg.startswith('-MF'): args[i] = '-MF' + str(scratch / (name + '.d'))
            if arg.startswith('-MT'): args[i] = '-MT' + str(scratch / (name + '.o'))
            if arg == '-o': args[i+1] = str(scratch / (name + '.o'))
        args[-1] = str(src)
        args[1:1] = ['-DS5_SIZE_ONLY', '-fstack-usage']
        run(args)
        output = run([nm, '-S', '--size-sort', str(scratch / (name + '.o'))])
        print('ARM', name, '\n', output)
    for usage in scratch.glob('*.su'):
        log.write(usage.name + '\n' + usage.read_text())
print(logpath)

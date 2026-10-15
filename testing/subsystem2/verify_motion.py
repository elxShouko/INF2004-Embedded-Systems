"""Buddy 2 host/ARM verification. Artifacts exist only in a temporary directory."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile
from datetime import datetime, timezone

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
BASE = ROOT / 'app_program/subsystem2'
WARN = ['-Wall', '-Wextra', '-Werror', '-pedantic', '-Wconversion',
        '-Wsign-conversion', '-Wshadow', '-Wstrict-prototypes',
        '-Wmissing-prototypes', '-Wformat=2', '-Wdouble-promotion', '-Wundef']
HOST_GCC = r'C:/msys64/ucrt64/bin/gcc.exe'
ARM_GCC = r'C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-gcc.exe'


def run(args, log, scratch, timeout=120, expected_failure=False):
    env = os.environ.copy()
    env['TEMP'] = env['TMP'] = str(scratch)
    log.write(subprocess.list2cmdline(args) + '\n')
    log.flush()
    try:
        result = subprocess.run(args, cwd=ROOT/'build_make', env=env,
                                capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        message = f'TIMEOUT after {timeout} seconds; EXIT 124\n'
        log.write(message)
        log.flush()
        print(message, end='')
        raise SystemExit(124)
    output = result.stdout + result.stderr
    log.write(output + f'EXIT {result.returncode}\n')
    log.flush()
    if expected_failure:
        if result.returncode == 0:
            raise SystemExit('Expected rejection did not occur')
    elif result.returncode:
        print(output)
        raise SystemExit(result.returncode)
    return output


def source_files():
    return sorted(BASE.glob('*.c'))


def host_flags(profile, analyze):
    flags = [HOST_GCC, '-std=c99', *WARN, '-DMOTION_CONFIG_HOST_TEST',
             '-I'+str(BASE), '-I'+str(HERE)]
    if analyze:
        flags += ['-fanalyzer']
    if profile == 'optimized':
        flags += ['-O2', '-fshort-enums']
    return flags


def host_checks(log, scratch, adapter_sources, timeout):
    # Exercise the real timeout handler, not a separate imitation.
    try:
        run([sys.executable, '-c', 'import time; time.sleep(10)'], log,
            scratch, timeout=0.1)
    except SystemExit as error:
        if error.code != 124:
            raise
    else:
        raise SystemExit('Timeout self-check failed')
    log.write('RUNNER timeout self-check PASS (expected EXIT 124)\n')
    for extra in ([], ['-DMOTION_CONFIG_HOST_TEST', '-D_PICO_RP2040_']):
        output = run([HOST_GCC, '-std=c99', '-fsyntax-only',
                      '-DMOTION_RTOS_HOST_TEST', *extra,
                      '-I'+str(BASE), str(HERE/'test_rtos.c')], log, scratch,
                     expected_failure=True)
        if 'must never target physical hardware' not in output:
            raise SystemExit('Unexpected guard diagnostic')
    # Focused mutations are temporary copies. Production files are never edited.
    mutations = [
        ('error_code', 'task_motion.c',
         '(g_task_motion_wake > 0) ? E_OK : g_task_motion_wake',
         '(g_task_motion_wake > 0) ? E_OK : E_LIMIT', [100]),
        ('wrong_delete', 'task_motion.c', 'tk_del_flg(g_task_motion_wake)',
         'tk_del_mtx(g_task_motion_mutex)', [101]),
        ('missing_exit', 'task_motion.c', 'tk_ext_tsk();', '(void)0;', [114,115]),
        ('missing_latch', 'task_motion.c',
         'tk_set_flg(g_task_motion_safety,cause)',
         'tk_set_flg(g_task_motion_safety,(cause & 0U))', [108]),
        ('always_stale', 'task_motion.c',
         'bool b_fresh = motion_api_now(&now_us) &&',
         'bool b_fresh = (motion_api_now(&now_us) && false) &&', [116]),
        ('skip_enable', 'drv_encoder.c',
         'b_ok = p_port->p_enable(p_port->p_context, ENC_CAPTURE_MASK);',
         'b_ok = false;', [110]),
        ('missing_pool_rollback', 'subsystem2.c',
         '(E_OK == tk_del_mpf(mpf_samples))',
         '(false && (E_OK == tk_del_mpf(mpf_samples)))', [123]),
    ]
    for name, source, old, new, cases in mutations:
        original = BASE/source
        text = original.read_text()
        if text.count(old) != 1:
            raise SystemExit('Mutation anchor is not unique: '+name)
        mutant = scratch/(name+'.c')
        mutant.write_text(text.replace(old, new))
        exe = scratch/(name+'.exe')
        sources = [mutant if p == original else p for p in adapter_sources]
        log.write(f'MUTATION {name}: {source}: {old!r} -> {new!r}\n')
        flags = host_flags('default', False) + ['-DMOTION_RTOS_HOST_TEST']
        run(flags + list(map(str,sources)) + ['-o',str(exe)], log, scratch)
        for case in cases:
            output = run([str(exe),str(case)], log, scratch, timeout,
                         expected_failure=True)
            if f'[TEST] S2-U{case} FAIL ' not in output:
                raise SystemExit('Mutant rejected for unexpected reason')
        log.write('MUTATION '+name+' DETECTED PASS\n')


def verify_host(opts, out, scratch):
    pure = [p for p in source_files()
            if p.name not in ('subsystem2.c','task_producer.c')]
    excluded = {'subsystem2_test.c','task_producer.c','motion_foundation_test.c',
                'test_motor.c','test_drive.c','drv_rp2040.c'}
    adapter = [p for p in source_files() if p.name not in excluded]
    adapter += [HERE/'test_rtos.c']
    for profile in ('default','optimized'):
        with (out/f'verify_{opts.label}_{profile}.log').open('x') as log:
            run([HOST_GCC,'--version'],log,scratch)
            exe = scratch/(profile+'.exe')
            flags = host_flags(profile, opts.analyze)
            run(flags + list(map(str,pure)) + ['-o',str(exe)],log,scratch)
            output = run([str(exe)],log,scratch,opts.host_timeout)
            ids = re.findall(r'^\[TEST\] (S2-U\d+) PASS ',output,re.M)
            if len(ids)!=95 or set(ids)!={f'S2-U{i:02}' for i in range(3,98)}:
                raise SystemExit('Expected 95 distinct pure groups')
            exe = scratch/('rtos_'+profile+'.exe')
            run(flags + ['-DMOTION_RTOS_HOST_TEST'] + list(map(str,adapter)) +
                ['-o',str(exe)],log,scratch)
            for case in range(98,135):
                output = run([str(exe),str(case)],log,scratch,opts.host_timeout)
                ids = re.findall(r'^\[TEST\] (S2-U\d+) PASS ',output,re.M)
                if ids != [f'S2-U{case}']:
                    raise SystemExit('Missing/duplicate adapter test ID')
            print(profile, 'PASS: 95 pure + 37 adapter/lifecycle groups')
    with (out/f'verify_{opts.label}_checks.log').open('x') as log:
        host_checks(log,scratch,adapter,opts.host_timeout)
    print('Timeout, host-only guards and 7 focused mutations PASS')


def verify_arm(opts, out, scratch):
    recipe = next(s for s in (HERE/'m2_make_recipes.txt').read_text().splitlines()
                  if s.startswith('arm-none-eabi-gcc '))
    with (out/f'verify_{opts.label}_arm.log').open('x') as log:
        run([ARM_GCC,'--version'],log,scratch)
        for smp in (0,1):
            for test in (0,1):
                for source in source_files():
                    args = shlex.split(recipe)
                    args[0] = ARM_GCC
                    args = [a for a in args if a!='-DCNF_SMP=1']
                    if smp:
                        args.insert(1,'-DCNF_SMP=1')
                    for i,a in enumerate(args):
                        if a.startswith('-DAPP_TEST='):
                            args[i]=f'-DAPP_TEST={test}'
                        if a.startswith('-MF'):
                            args[i]='-MF'+str(scratch/(source.stem+'.d'))
                        if a.startswith('-MT'):
                            args[i]='-MT'+str(scratch/(source.stem+'.o'))
                        if a=='-o':
                            args[i+1]=str(scratch/(source.stem+'.o'))
                    args[-1]=str(source)
                    args.insert(1,'-fstack-usage')
                    inherited = {'subsystem2.c','subsystem2_test.c',
                                 'task_producer.c','motion_foundation_test.c',
                                 'test_motor.c'}
                    if opts.strict and source.name not in inherited:
                        args = ['-isystem'+a[2:] if a.startswith('-I') else a
                                for a in args]
                        args[1:1] = WARN
                    run(args,log,scratch)
                print('ARM SMP',smp,'TEST',test,'PASS',len(source_files()),'objects')


def main():
    parser = argparse.ArgumentParser()
    for flag in ('arm','strict','analyze','style'):
        parser.add_argument('--'+flag,action='store_true')
    parser.add_argument('--label',default=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S'))
    parser.add_argument('--output-dir',type=Path,default=HERE/'logs')
    parser.add_argument('--host-timeout',type=float,default=5.0)
    opts = parser.parse_args()
    if not re.fullmatch(r'[A-Za-z0-9_]+',opts.label):
        parser.error('label must contain only letters, digits or underscores')
    if not (0 < opts.host_timeout <= 60):
        parser.error('host timeout must be >0 and <=60 seconds')
    if opts.style:
        names = {'enc_capture','drv_encoder','motion_bus','motion_run','move_plan',
                 'wheel_loop','task_motion','motion_api','test_drive','test_rtos'}
        paths = sorted(p for root in (BASE,HERE) for p in root.iterdir()
                       if p.stem in names and p.suffix in ('.c','.h'))
        problems = []
        for p in paths:
            text = p.read_text()
            if not text.rstrip().endswith('/*** end of file ***/'):
                problems.append(p.name+': missing EOF marker')
            for i,line in enumerate(text.splitlines(),1):
                if len(line)>80 or '\t' in line or line.rstrip()!=line:
                    problems.append(f'{p.name}:{i}: width/tab/trailing space')
        stems={p.stem for p in paths}
        if len({n[:8] for n in stems})!=len(stems):
            problems.append('first-eight-character filename collision')
        print('\n'.join(problems) if problems else f'Style subset PASS {len(paths)} files')
        return bool(problems)
    out = opts.output_dir.resolve()
    evidence_root = (HERE/'logs').resolve()
    if out != evidence_root and evidence_root not in out.parents:
        parser.error('evidence must stay inside testing/subsystem2/logs')
    out.mkdir(parents=True,exist_ok=True)
    paths = sorted(p for root in (BASE,HERE) for p in root.iterdir()
                   if p.suffix in ('.c','.h','.py') or p.name=='m2_make_recipes.txt')
    metadata = {'head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                'working_tree':subprocess.check_output(['git','status','--short'],cwd=ROOT,text=True),
                'utc_start':datetime.now(timezone.utc).isoformat(),
                'options':vars(opts)|{'output_dir':str(out)},
                'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
                                 for p in paths}}
    with (out/f'verify_{opts.label}_metadata.json').open('x') as record:
        json.dump(metadata,record,indent=2)
        record.write('\n')
    # Existing shared 'build' ignore rule covers this Buddy 2-local scratch.
    # Keep it in the workspace: sandbox temp paths are inaccessible to GCC.
    scratch_root = HERE/'build'
    if scratch_root.resolve() != HERE.resolve()/'build':
        parser.error('scratch directory must not redirect outside Buddy 2')
    scratch_root.mkdir(exist_ok=True)
    # Remove only this invocation's private temporary directory. Another run
    # can concurrently use the ignored parent; never remove its scratch files.
    with tempfile.TemporaryDirectory(prefix='verify_',dir=scratch_root) as tmp:
        scratch = Path(tmp)
        if opts.arm:
            verify_arm(opts,out,scratch)
        else:
            verify_host(opts,out,scratch)
    return 0


if __name__ == '__main__':
    sys.exit(main())

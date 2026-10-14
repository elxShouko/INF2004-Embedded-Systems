"""Buddy 2-local host/ARM object verification; never executes firmware."""
from pathlib import Path
import argparse, os, shlex, subprocess, sys
BASE=Path(__file__).resolve().parent
ROOT=BASE.parents[1]
WARN=['-Wall','-Wextra','-Werror','-pedantic','-Wconversion',
      '-Wsign-conversion','-Wshadow','-Wstrict-prototypes',
      '-Wmissing-prototypes','-Wformat=2','-Wdouble-promotion','-Wundef']
def run(args,log):
    env=os.environ.copy();env['TEMP']=str(BASE);env['TMP']=str(BASE)
    result=subprocess.run(args,cwd=ROOT/'build_make',env=env,
                          capture_output=True,text=True)
    log.write(subprocess.list2cmdline(args)+'\n'+result.stdout+result.stderr+
              f'EXIT {result.returncode}\n');log.flush()
    if result.returncode:
        print(result.stdout+result.stderr);raise SystemExit(result.returncode)
    return result.stdout
parser=argparse.ArgumentParser();parser.add_argument('--arm',action='store_true')
parser.add_argument('--strict',action='store_true')
parser.add_argument('--analyze',action='store_true')
parser.add_argument('--style',action='store_true')
parser.add_argument('--label',default='current');opts=parser.parse_args()
assert opts.label.replace('_','').isalnum()
if opts.style:
    names=['enc_capture','drv_encoder','motion_bus','motion_run','move_plan',
           'wheel_loop','task_motion','motion_api','test_drive']
    checked=[]; problems=[]
    for name in names:
        for path in BASE.glob(name+'.*'):
            if path.suffix not in ('.c','.h'):continue
            checked.append(path)
            data=path.read_text()
            if not data.rstrip().endswith('/*** end of file ***/'):
                problems.append(f'{path.name}: missing EOF marker')
            for index,line in enumerate(data.splitlines(),1):
                if len(line)>80 or '\t' in line or line.rstrip()!=line:
                    problems.append(f'{path.name}:{index}: width/tab/trailing space')
    stems={p.stem for p in checked}
    if len({n[:8] for n in stems})!=len(stems):
        problems.append('first-eight-character module filename collision')
    print('\n'.join(problems) if problems else f'Style subset PASS {len(checked)} files')
    raise SystemExit(bool(problems))
if not opts.arm:
    gcc=r'C:\msys64\ucrt64\bin\gcc.exe'
    for profile in ['default','optimized']:
        exe=BASE/f'verify_generated_{profile}.exe'
        with (BASE/f'verify_{opts.label}_{profile}.log').open('w') as log:
            args=[gcc,'-std=c99',*WARN,'-DMOTION_CONFIG_HOST_TEST']
            if opts.analyze:args+=['-fanalyzer']
            if profile=='optimized':args+=['-O2','-fshort-enums']
            args += [str(p) for p in BASE.glob('*.c') if p.name not in ('subsystem2.c','task_producer.c')]+['-o',str(exe)]
            run(args,log); output=run([str(exe)],log)
            print(profile,'PASS groups',output.count(' PASS '))
else:
    gcc=r'C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\gcc-arm-none-eabi\bin\arm-none-eabi-gcc.exe'
    recipe=next(s for s in (BASE/'m2_make_recipes.txt').read_text().splitlines()
                if s.startswith('arm-none-eabi-gcc '))
    with (BASE/f'verify_{opts.label}_arm.log').open('w') as log:
        for smp in [0,1]:
            for test in [0,1]:
                for source in BASE.glob('*.c'):
                    args=shlex.split(recipe);args[0]=gcc
                    args=[a for a in args if a!='-DCNF_SMP=1']
                    if smp:args.insert(1,'-DCNF_SMP=1')
                    for i,a in enumerate(args):
                        if a.startswith('-DAPP_TEST='):args[i]=f'-DAPP_TEST={test}'
                        if a.startswith('-MF'):args[i]='-MF'+str(BASE/f'verify_generated_{source.stem}.d')
                        if a.startswith('-MT'):args[i]='-MT'+str(BASE/f'verify_generated_{source.stem}.o')
                        if a=='-o':args[i+1]=str(BASE/f'verify_generated_{source.stem}.o')
                    args[-1]=str(source)
                    args.insert(1,'-fstack-usage')
                    if opts.strict and source.name not in ('subsystem2.c','subsystem2_test.c','task_producer.c','motion_foundation_test.c','test_motor.c'):
                        # Supplied RTOS headers preserve their original GNU
                        # conventions; warning checks apply to application code.
                        args=[('-isystem'+a[2:]) if a.startswith('-I') else a for a in args]
                        args[1:1]=WARN
                    run(args,log)
                print('ARM SMP',smp,'TEST',test,'PASS')


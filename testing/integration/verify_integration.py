"""Shared integration verification: bounded tools, existing ignored build scratch."""
from pathlib import Path
import argparse, hashlib, json, os, re, runpy, shlex, subprocess, tempfile
from datetime import datetime, timezone
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
TOOLS=runpy.run_path(str(ROOT/'testing/subsystem2/verify_motion.py'))
BASE=ROOT/'app_program'
PRODUCTION=[BASE/n for n in ['vehicle.c','vehicle_rt.c','gpio_bank.c','app_main.c',
 'subsystem1/s1_codec.c','subsystem2/motion_config.c','subsystem2/motion_quad.c',
 'subsystem2/motion_estimator.c','subsystem2/motion_pid.c','subsystem2/drv_motor.c',
 'subsystem2/enc_capture.c','subsystem2/wheel_loop.c','subsystem2/move_plan.c',
 'subsystem2/motion_run.c','subsystem3/drv_line_tracking.c',
 'subsystem3/drv_ir_input.c','subsystem3/drv_navigation_map.c',
 'subsystem4/imu_core.c','subsystem4/imu_guard.c','subsystem5/s5_obstacle_logic.c']]
def run(args,log,timeout=120):
    log.write(subprocess.list2cmdline(args)+'\n');log.flush()
    try:r=subprocess.run(args,cwd=ROOT/'build_make',capture_output=True,text=True,timeout=timeout)
    except subprocess.TimeoutExpired:
        log.write('TIMEOUT EXIT 124\n');log.flush();raise SystemExit(124)
    log.write(r.stdout+r.stderr+f'EXIT {r.returncode}\n');log.flush()
    if r.returncode:print(r.stdout+r.stderr);raise SystemExit(r.returncode)
    return r.stdout

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--label',required=True)
    parser.add_argument('--arm',action='store_true')
    opts=parser.parse_args()
    if not re.fullmatch('[A-Za-z0-9_]+',opts.label):parser.error('invalid label')
    logs=HERE/'logs';logs.mkdir(exist_ok=True)
    files=sorted([p for p in BASE.rglob('*') if p.suffix in ('.c','.h')]+
        [HERE/'test_ports.h',HERE/'test_vehicle.c',Path(__file__).resolve(),
         ROOT/'testing/subsystem2/test_rtos.h',ROOT/'testing/subsystem2/verify_motion.py',
         ROOT/'testing/subsystem2/m2_make_recipes.txt'])
    with (logs/f'{opts.label}_metadata.json').open('x') as f:
        json.dump({'head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
            'branch':subprocess.check_output(['git','branch','--show-current'],cwd=ROOT,text=True).strip(),
            'status':subprocess.check_output(['git','status','--short'],cwd=ROOT,text=True),
            'utc':datetime.now(timezone.utc).isoformat(),'arm':opts.arm,
            'sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},
            # Raw hashes describe compiler inputs; blob IDs bind evidence to Git's
            # configured clean/line-ending normalization for the exact commit.
            'git_blob':{p.relative_to(ROOT).as_posix():subprocess.check_output(
                ['git','hash-object','--path='+p.relative_to(ROOT).as_posix(),str(p)],
                cwd=ROOT,text=True).strip() for p in files}},f,indent=2)
    scratch=HERE/'build'
    if scratch.resolve()!=HERE.resolve()/'build':raise SystemExit('unsafe scratch redirect')
    scratch.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='integration_',dir=scratch) as tmp:
        temp=Path(tmp)
        os.environ['TMP']=str(temp)
        os.environ['TEMP']=str(temp)
        if opts.arm:
            recipe=next(s for s in (ROOT/'testing/subsystem2/m2_make_recipes.txt').read_text().splitlines() if s.startswith('arm-none-eabi-gcc '))
            with (logs/f'{opts.label}_arm.log').open('x') as log:
                run([TOOLS['ARM_GCC'],'--version'],log)
                for smp in (0,1):
                    for test in (0,1):
                        for source in PRODUCTION[:4]:
                            args=shlex.split(recipe);args[0]=TOOLS['ARM_GCC']
                            args=[a for a in args if a!='-DCNF_SMP=1']
                            if smp:args.insert(1,'-DCNF_SMP=1')
                            for i,a in enumerate(args):
                                if a.startswith('-DAPP_TEST='):args[i]=f'-DAPP_TEST={test}'
                                if a.startswith('-MF'):args[i]='-MF'+str(temp/(source.stem+'.d'))
                                if a.startswith('-MT'):args[i]='-MT'+str(temp/(source.stem+'.o'))
                                if a=='-o':args[i+1]=str(temp/(source.stem+'.o'))
                            args[-1]=str(source)
                            args=[('-isystem'+a[2:]) if a.startswith('-I') else a for a in args]
                            args[1:1]=TOOLS['WARN']+['-fstack-usage']
                            run(args,log)
                            for usage in temp.glob('*.su'):
                                log.write(usage.read_text())
                                usage.unlink()
                        print('ARM',smp,test,'4 shared objects PASS')
        else:
            for profile in ('default','optimized'):
                with (logs/f'{opts.label}_{profile}.log').open('x') as log:
                    run([TOOLS['HOST_GCC'],'--version'],log)
                    flags=[TOOLS['HOST_GCC'],'-std=c99',*TOOLS['WARN'],'-fanalyzer',
                        '-DVEHICLE_HOST_TEST','-DMOTION_CONFIG_HOST_TEST',
                        '-DMOTION_RTOS_HOST_TEST','-DAPP_TEST=0','-I'+str(BASE),
                        '-I'+str(HERE),'-include',str(HERE/'test_ports.h')]
                    if profile=='optimized':flags+=['-O2','-fshort-enums']
                    exe=temp/(profile+'.exe')
                    objects=[]
                    for index,source in enumerate(PRODUCTION+[HERE/'test_vehicle.c']):
                        obj=temp/f'{profile}_{index}.o'
                        options=flags.copy()
                        if source.name=='s1_codec.c':
                            # Existing provider warnings remain visible; do not rewrite owner code.
                            options+=['-Wno-error=conversion','-Wno-error=sign-conversion']
                        run(options+['-c',str(source),'-o',str(obj)],log)
                        objects.append(str(obj))
                    run([TOOLS['HOST_GCC'],*objects,'-lm','-o',str(exe)],log)
                    for case in range(3,48):
                        output=run([str(exe),str(case)],log,5)
                        if re.findall(r'^\[TEST\] (INT-\d+) PASS ',output,re.M)!=[f'INT-{case:02}']:
                            raise SystemExit('wrong/missing test ID')
                    print(profile,'45 integration groups PASS')
if __name__=='__main__':main()

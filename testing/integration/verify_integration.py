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
         ROOT/'testing/subsystem2/m2_make_recipes.txt']+
        [ROOT/'testing/subsystem5/host'/n for n in
         ('test_obstacle_logic.c','test_obstacle_runtime.c',
          'test_s5_port.c','test_s5_port.h')])
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
                        for source in PRODUCTION[:4]+[BASE/'subsystem1/s1_codec.c']+[
                            BASE/'subsystem5'/name for name in ['s5_acquire.c',
                            'drv_s5.c','s5_live.c','subsystem5.c',
                            's5_obstacle_runtime.c','s5_obstacle_logic.c']]:
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
                        print('ARM',smp,test,'11 integration/provider objects PASS')
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
                    for case in list(range(3,63))+list(range(64,82)):
                        output=run([str(exe),str(case)],log,5)
                        if re.findall(r'^\[TEST\] (INT-\d+) PASS ',output,re.M)!=[f'INT-{case:02}']:
                            raise SystemExit('wrong/missing test ID')
                    print(profile,'78 normal integration groups PASS')
                    testobjects=objects.copy()
                    testflags=[a for a in flags if a!='-DAPP_TEST=0']+['-DAPP_TEST=1']
                    for index in (1,3):
                        obj=temp/f'{profile}_test_{index}.o'
                        run(testflags+['-c',str(PRODUCTION[index]),'-o',str(obj)],log)
                        testobjects[index]=str(obj)
                    testexe=temp/'test_startup.exe'
                    run([TOOLS['HOST_GCC'],*testobjects,'-lm','-o',str(testexe)],log)
                    output=run([str(testexe),'63'],log,5)
                    if '[TEST] INT-63 PASS ' not in output:
                        raise SystemExit('TEST=1 startup isolation failed')
                    # Deliberately regress only the shared adapter in scratch.
                    # Each check must be rejected by its production-path case.
                    runtime=(BASE/'vehicle_rt.c').read_text()
                    mutations=[('s1_net_estop_latched() ||', 'false ||',48),
                        ('ER status_error = s3_get_status(&status);',
                         'ER status_error = E_TMOUT;',49),
                        ('g_vehicle_started = false; /* Failed start',
                         'g_vehicle_started = true; /* Failed start',52),
                        ('ER event_error = imu_runtime_get_event(&event);',
                         'ER event_error = E_TMOUT;',54),
                        ('p_frame->encoder_count_left = p_input->motion.encoder_count[0];',
                         'p_frame->encoder_count_left = (int32_t)p_input->motion.encoder_count[0];',61),
                        ('if (!p_clearance->b_verified || !p_clearance->b_original_line_verified ||',
                         'if (false || !p_clearance->b_original_line_verified ||',67),
                        ('const bool b_success = b_search ? (p_vehicle->b_route_found &&',
                         'const bool b_success = b_search ? (true &&',72),
                        ('p_input->obstacle_started_us < p_vehicle->route_ready_raw_us',
                         'false',77)]
                    for old,new,case in mutations:
                        text=((BASE/'subsystem5/s5_obstacle_logic.c').read_text()
                              if case==67 else ((BASE/'vehicle.c').read_text()
                              if case in (61,72,77) else runtime))
                        if text.count(old)!=1:raise SystemExit('mutation anchor')
                        source=temp/'mutated_runtime.c'
                        source.write_text(text.replace(old,new))
                        obj=temp/'mutated_runtime.o'
                        run(flags+['-I'+str(BASE/'subsystem5'),'-c',str(source),'-o',str(obj)],log)
                        altered=objects.copy()
                        altered[19 if case==67 else (0 if case in (61,72,77) else 1)]=str(obj)
                        mutant=temp/'mutant.exe'
                        run([TOOLS['HOST_GCC'],*altered,'-lm','-o',str(mutant)],log)
                        result=subprocess.run([str(mutant),str(case)],
                            capture_output=True,text=True,timeout=5)
                        log.write(f'MUTATION INT-{case:02}\n'+result.stdout+
                            result.stderr+f'EXPECTED EXIT 1; EXIT {result.returncode}\n')
                        if result.returncode!=1:raise SystemExit('undetected mutation')
                    print(profile,'8 shared/telemetry/route mutations detected')
                    s5=BASE/'subsystem5'
                    s5test=ROOT/'testing/subsystem5/host/test_obstacle_runtime.c'
                    s5exe=temp/'s5_live.exe'
                    run(flags+['-DS5_LIVE_HOST_TEST',str(s5test),
                        str(s5/'s5_acquire.c'),str(s5/'s5_obstacle_logic.c'),
                        str(s5/'s5_obstacle_runtime.c'),str(s5/'s5_live.c'),
                        '-lm','-o',str(s5exe)],log)
                    run([str(s5exe)],log)
                    logicexe=temp/'s5_logic.exe'
                    run(flags+[str(ROOT/'testing/subsystem5/host/test_obstacle_logic.c'),
                        str(s5/'s5_obstacle_logic.c'),'-o',str(logicexe)],log)
                    run([str(logicexe)],log)
                    for case in list(range(58,64))+[86]:run([str(s5exe),str(case)],log,5)
                    portexe=temp/'s5_port.exe'
                    portflags=[TOOLS['HOST_GCC'],'-std=c99',*TOOLS['WARN'],
                        '-fanalyzer','-DS5_DRIVER_HOST_TEST','-D_PICO_RP2040_',
                        '-DCNF_SMP=1',
                        '-I'+str(ROOT/'include'),'-I'+str(ROOT/'config'),
                        '-I'+str(ROOT/'testing/subsystem5/host')]
                    if profile=='optimized':portflags+=['-O2','-fshort-enums']
                    run(portflags+[str(s5/'drv_s5.c'),
                        str(s5/'s5_acquire.c'),str(s5/'s5_obstacle_logic.c'),
                        str(ROOT/'testing/subsystem5/host/test_s5_port.c'),
                        '-o',str(portexe)],log)
                    run([str(portexe)],log)
                    print(profile,'S5 policy/acquisition/live/MMIO cases PASS')
if __name__=='__main__':main()

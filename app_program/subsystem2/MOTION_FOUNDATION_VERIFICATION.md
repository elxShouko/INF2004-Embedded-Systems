# Motion foundation correction verification — 2026-10-14

This is a Buddy 2-local correction record. Shared testing/ records were not
modified because approval is restricted to app_program/subsystem2/.
Synthetic fixtures are not robot calibrations or stability evidence.

## Revision and scope

Branch: buddy2-Motor-and-encoder-control.
HEAD and local main: 2886fa8a0118185674f59964ab832d7c4e3ceabb.
Tests exercise the uncommitted working tree, including untracked foundation
sources. Nothing was staged, committed, pushed or switched.

The correction changes motion_pid.c/.h, motion_estimator.c/.h and
motion_foundation_test.c. Formatting-only changes apply to motion_config.c/.h,
motion_quad.c/.h and motion_types.h. subsystem2_test.c is byte-identical to the
starting working tree; its existing M1A edits are preserved.

## PID correction

The previous algorithm rejected an entire integral step if its candidate effort
crossed an output limit. A large pure-I step could stall indefinitely; an
overspeed correction could retain positive effort instead of unwinding it.

Let B = P + D, I0 be the old integral (after target-reversal handling), and
T = clamp(I0 + Ki*error*dt, integral_min, integral_max).

- If T > I0 and B + T > output_max:
  I1 = max(I0, min(T, output_max - B)).
- If T < I0 and B + T < output_min:
  I1 = min(I0, max(T, output_min - B)).
- Otherwise I1 = T.
- Effort = clamp(B + I1, output_min, output_max).

I1 stays between the previous integral and the bounded candidate, hence within
explicit integral bounds. This permits reaching and crossing output boundaries
without additional saturation-driving accumulation. It does not inject an
opposite integral solely to cancel P/D saturation. All arithmetic remains
subject to finite checks. Existing reset, idle, zero-target, reversal and
arithmetic-fault disabling semantics are retained. No new gain is introduced.

## Estimator correction

A nonnull object with a null required sample/output records NULL, clears cached
speed and its validity, and preserves position, historical timestamp, fractional
remainder, accepted count/time/generation baseline and reset latch. A subsequent
valid sample uses the last accepted baseline and the existing timestamp/reset
rules. A null object is safely rejected; a provided output is cleared.

## Regression evidence

Before changing production behaviour, new S2-U36/U37/U38 failed against the old
implementation; all original 33 host groups passed. New U39/U40/U41 already
passed. After correction, both final compiler profiles pass all 40 host groups:
10 M1A groups plus 30 M1B–M1D groups, with zero failures.

| ID | Added coverage | Final result, both profiles |
|---|---|---|
| S2-U36 | Large pure-I steps, both signs, repeated saturation and recovery | PASS |
| S2-U37 | Accumulated integral, overspeed crossing, both signs, repeated recovery | PASS |
| S2-U38 | Null sample/output, cached error, baseline/remainder preservation, recovery | PASS |
| S2-U39 | ±1.666 and ±3.333 mm/s truncation, units, flags, exact/max+1 timing | PASS |
| S2-U40 | NaN/±infinity in all four bounds; invalid finite bounds | PASS |
| S2-U41 | Nonfinite derivative, integral sum and effort sum; disabled retry | PASS |
| S2-U42 | P/D headroom, asymmetric output limits and tighter integral bounds | PASS |

Original private foundation-test helper bodies and original report calls were
compared with the pre-edit snapshot: unchanged. The fractional-position/reversal
walk and all original assertions remain. No on-target demonstration tests ran.

## Reproduction commands

Compiler: C:/msys64/ucrt64/bin/gcc.exe, MSYS2 GCC 14.2.0 (Rev2).
From the repository root in PowerShell:

```powershell
$env:TEMP = (Resolve-Path app_program/subsystem2).Path
$env:TMP = $env:TEMP
$motionSources = @('subsystem2_test.c','motion_config.c','motion_quad.c',
    'motion_estimator.c','motion_pid.c','motion_foundation_test.c') |
    ForEach-Object { 'app_program/subsystem2/' + $_ }
$motionWarnings = @('-Wall','-Wextra','-Werror','-pedantic','-Wconversion',
    '-Wsign-conversion','-Wshadow','-Wstrict-prototypes','-Wmissing-prototypes',
    '-Wformat=2','-Wdouble-promotion','-Wundef')
& C:/msys64/ucrt64/bin/gcc.exe -std=c99 @motionWarnings `
    -DMOTION_CONFIG_HOST_TEST @motionSources `
    -o app_program/subsystem2/motion_audit_host.exe
& ./app_program/subsystem2/motion_audit_host.exe
& C:/msys64/ucrt64/bin/gcc.exe -std=c99 -O2 -fshort-enums @motionWarnings `
    -DMOTION_CONFIG_HOST_TEST @motionSources `
    -o app_program/subsystem2/motion_audit_opt.exe
& ./app_program/subsystem2/motion_audit_opt.exe
```

Both compiles emitted no diagnostics; each executable exited zero.
GCC -fanalyzer compiled each of the six translation units with -std=c99 -O2
-Wall -Wextra -Werror -pedantic -DMOTION_CONFIG_HOST_TEST -fanalyzer -c,
with temporary output objects inside Buddy 2: no diagnostics.
An initial analyzer invocation used an incorrect relative TEMP path and failed;
it was corrected before the successful six-file analyzer run.
The -ffast-math negative compilation check failed at the intended header #error,
confirming rejection of a profile that invalidates finite checks.
No sanitizer or integrated ARM build was run.

## Barr-C and formatting

Production declarations/definitions now use the prescribed function-name space;
compound logical operands were parenthesized; public bodies precede private
bodies; production files have EOF markers and trailing blank lines. Checks passed
for tabs, trailing whitespace, 80-column limits, declaration spacing, EOF markers
and the 100-line function bound. git diff --check passed for tracked changes.
This is focused compliance checking, not a formal Barr-C certification.

Remaining deviations/limitations for review:

- motion_pid_value_t retains a module-prefixed native float alias with binary32
  guards; C99 does not supply a standard float32_t. Rationale is recorded in the
  header. Software floating-point and nontrapping behaviour need ARM validation.
- Existing host-only main remains in subsystem2_test.c; test adapter macros,
  T-Monitor UB* format casts, and inherited test layout are retained to preserve
  professor conventions. No competing firmware main is introduced.
- Function documentation is not uniformly Doxygen-tagged; test code has not been
  comprehensively restyled. These are not claimed as fully resolved deviations.

## Ownership and remaining verification

A before/after SHA-256 inventory found no changed or removed files outside
app_program/subsystem2/. Existing demonstration implementation and other buddies
are unchanged. Root AGENTS.md and reference_docs/ were already untracked and
remain untouched. Generated host executables and analyzer objects were removed
using explicit, workspace-checked paths.

No remaining High/Critical software finding was identified in this correction
pass. The existing Make wildcard discovers these modules without shared edits.
The foundation is ready for a separately authorised ARM build, not actuation.
ARM compile/link, code size, stack, software-float behaviour, timing/SMP handling,
hardware calibration, pin conflicts and reviewed stop/integration contracts
remain unverified. No motor GPIO/PWM, encoder IRQ or RTOS integration was added.

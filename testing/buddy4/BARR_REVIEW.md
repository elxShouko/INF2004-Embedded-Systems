# Barr-C:2018 review scope

New application modules are `drv_imu`, `drv_lsm`, `imu_core`, `imu_runtime`,
`imu_config`, and the updated S4 lifecycle. New pure logic and test doubles
compile as C99 with `-Wall -Wextra -Werror -Wconversion -Wshadow -pedantic`.
Integrated firmware retains the professor template's GNU C11 settings.
Compiler warnings and sanitizers are checks, not a Barr certification.

Applied: four spaces, Allman braces, fixed-width storage and unsigned bit
operations, explicit float32_t and `f` constants, finite/range checks, no
implementation-defined negative shifts, static module internals, module-prefixed
public APIs, headers/guards, one-owner state, bounded buffers, explicit errors,
and no new ISR, goto, continue, longjmp, recursion or dynamic application heap.
Functions are kept below 100 source lines. Hardware addresses and conversions
are tied to the sensor source; development thresholds are labelled as such.
EOF marker comments retain the standard's following blank line. Git's
blank-at-EOF whitespace warning is excluded explicitly when checking diffs;
all other configured whitespace checks remain enabled.

Required/inherited conventions are documented, not silently restyled:

- Kernel `ER`, `ID`, `T_C*`, `FP`, `EXPORT`, `IMPORT`, task signatures and
  `tk_*` names remain the supplied API. FP casts follow its handler convention.
- The C-library bridge includes stddef before the kernel with
  `PROHIBIT_DEF_SIZE_T`; this avoids conflicting size_t declarations.
- Pico SDK types, int return values, and memory-mapped volatile registers use
  SDK conventions. Those volatile accesses are not a synchronization method.
- Published legacy `mbf_summary`, `flg_events`, `alm_monitor`,
  `s4_lines_received` and monitor task names remain unchanged for callers.
- `task_monitor.c` and its historical checks retain template style and behavior;
  their existing unguarded counter, unchecked API calls and monitor/LED demo
  are **not newly certified production code**. Their retirement requires
  coordinated interface work. No kernel/platform source was restyled.
- Host `main`, conventional argc/argv, and fake kernel API names follow their
  external contracts. Host entry points have no redundant matching header;
  fake API structure names deliberately mirror the kernel rather than `_t`.
- SDK and kernel implementations are outside this application-style review.

Review the retained template exceptions with the team before claiming whole
subsystem compliance. The new implementation does not claim that host tests
establish physical accuracy, deterministic timing or multicore correctness.


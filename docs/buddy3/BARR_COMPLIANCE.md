# Member 3 Barr-C:2018 review

Baseline: `a508994331acf0adaa9acff1927cdaeb9f232abe`, branch
`buddy3-barcode`. The changes are an uncommitted application-only working tree.
The authoritative standard is the locally supplied
`reference_docs/standards/barr_c_coding_standard_2018.pdf`.

## Status

This is a compliance cleanup with **approved compatibility exceptions**, not a
claim of unconditional compliance or an independently certified audit. The
requesting user explicitly approved S3-DEV-01 through S3-DEV-03 in this chat on
2026-10-15, answering: "Approve the documented compatibility exceptions."
That approval is limited to the entries below and does not authorize unrelated
shared-file changes, commits, pushes or hardware access.

The scope is `app_program/subsystem3/`. Other subsystem implementations, shared
startup, kernel/configuration, build files and standalone copies were not
changed. Application tests/records are under `testing/subsystem3/`.

## Changes and checked rules

- Matching driver source/header pairs now live together. Existing `line/`
  headers forward to those pairs so callers retain their include paths.
- Files use LF, spaces, four-column indentation, Allman control braces,
  80-column lines, aligned declarations and explicit end-of-file markers.
- Definitions have the required space before their argument parentheses.
- Driver loop counters are descriptive, fixed-width unsigned values; protocol
  sizes and bounded-work limits have names rather than unexplained loop bounds.
- Module/function comments are Doxygen-compatible. Cast assumptions are
  recorded beside the applicable function. Numerical width operations widen
  before bit manipulation or checked narrowing.
- Multiple local declarators were split, public/protected resource visibility
  was retained, private functions remain static, and source sections separate
  declarations/prototypes/public bodies/private bodies.
- The 152-line processor was split into sampling, line-update, loss, recovery,
  barcode and junction stages, retaining the original execution order.
- Nested input/recovery/capture work was split into helpers. Functions are
  limited to 100 physical lines in the checked snapshot.
- Infinite loops use `for (;;)`, switch breaks align with case labels, terminal
  else branches document intentional no-ops, and simple equality tests place
  constants first.
- Signed kernel receive sizes are compared with explicitly range-documented
  signed sizes. Mode validation avoids the ARM compiler's always-true unsigned
  lower-bound comparison while rejecting invalid mode representations.
- Shared status and exported kernel handles are volatile-qualified where read
  by tasks; existing mutexes remain the synchronization mechanism. Volatile is
  not being used as a substitute for multicore locking.

`testing/subsystem3/check_barr_style.py` checks a declared subset: line width,
tabs/line endings, module/function comments, EOF markers, include guards,
paired headers, short index variables, infinite loops and function spans.
It does not parse C semantics or establish complete rule coverage.

## Approved compatibility exceptions

| ID | Rules / affected scope | Preserved requirement and reason | Approval |
| --- | --- | --- | --- |
| S3-DEV-01 | 4.1.b, 5.1, 6.1.i, 7.1.j/n; template entry/demo interface | Preserve `subsystem3_init/start/test`, `consumer_task`, `APP_STATS`, `mbx_samples`, `mtx_stats`, `app_stats`, and the required subsystem filenames. Shared startup and S2/S4 consume these names. Renaming them requires coordinated shared/other-owner edits. | Requesting user, 2026-10-15 |
| S3-DEV-02 | 4.2.c.i preference, 5.1.c, 6.1.i, 7.1.j/n and Boolean-member naming; existing public S3 contract | Preserve `s3_*`, `ir_*`, `barcode_*`, `line_*`, `nav_*`, `junction_*`, public struct member names and the published queue/status/task IDs. Their prefixes do not literally match every `drv_*` filename, and exported IDs lack Barr global/handle prefixes. Compatibility headers preserve old includes. No caller renaming, memory-layout change, queue-format change or alias macros are hidden in this cleanup. | Requesting user, 2026-10-15 |
| S3-DEV-03 | 1.1.a and required platform conventions | Shared makefiles select GNU C11, and the RTOS requires its own `ID`, `ER`, `INT`, `FP`, attributes and callback conventions. New algorithm source is also checked as C99; shared compiler flags and supplied internals are outside this change. The source-local `PROHIBIT_DEF_SIZE_T` bridge preserves the port's C-library integration. | Requesting user, 2026-10-15 |

The number/naming exceptions are not requests to rewrite the supplied kernel.
They now form the approved scoped compatibility policy for this cleanup.
Any later removal of these exceptions requires coordinated renames and caller
updates. Draft source comments saying "under review" or "pending team review"
describe the pre-approval review state; this dated approval record supersedes
those comments without changing the verified C/header source fingerprint.

The preference for one return per function (6.2.c) is not applied to validation
guards: early returns preserve the established error paths without deep nesting.
The retained demo's error-path `continue` follows its existing control flow;
1.7.d expresses a preference to avoid it. Neither choice changes the queue or
motor ownership policy.

The fake-kernel host test intentionally includes its implementation to exercise
private owner state. That test harness is not production application source;
it must not be presented as compliant with production rule 4.3.f.

## Verification and limitations

Documentation was grouped under `docs/buddy3/` on 2026-10-15. The review was
previously at `app_program/subsystem3/BARR_COMPLIANCE.md`. Source comment paths
were updated to the new locations; executable statements were unchanged.
Historical test fingerprints below identify the pre-relocation source comments,
not the current documentation-only working tree.

Stable test IDs S3-U02 through S3-U08 and exact run evidence are recorded in
`testing/subsystem3/TEST_LOG.md` and its logs. Each run identifies the baseline
and the uncommitted source fingerprint instead of claiming an uncreated commit.
No firmware is flashed. No sensor, motor, on-target timing or SMP validation is
inferred from a host test or build. Calibration, team integration and the
existing missing Mission Controller remain separate work.

The 2026-10-15 run passed all four before/after host suites, public-layout
comparison, a 40,000-sample differential state trace, ARM C99 syntax checks and
both integrated Make profiles. The mechanical checker covered 26 C/header
files; the longest function was 88 physical lines. See the test log for the
tested uncommitted fingerprint rather than attributing results to HEAD alone.

The whitespace diff check uses `git -c core.whitespace=-blank-at-eof diff --check`.
Only Git's blank-at-EOF warning is exempted because Barr rule 3.3.c explicitly
requires an end-of-file comment followed by a blank line. Other whitespace
checks remain enabled.

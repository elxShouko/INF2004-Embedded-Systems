# Buddy 3 documentation

Buddy 3 owns IR sensing and calibration, line observations, junction detection,
barcode recognition and navigation events. Its integrated source is in
`app_program/subsystem3/`.

| Document | Purpose |
| --- | --- |
| [Feedback audit and contract](feedback-audit-and-contract.md) | Current feedback disposition, pacing/ID fixes, sensor/mapping/recovery contract and pending target evidence |
| [Minimum-bit representation](minimum-bit-representation.md) | Current packed formats, measured RAM/message sizes and owner handoffs |
| [Member 3 guide](member3.md) | Algorithms, sensor mounting, standalone diagnostic use and limitations |
| [Template integration](member3-template-integration.md) | Integrated task lifecycle, configuration, messages and connections required from other subsystems |
| [Production timing and handoff](production-timing-and-handoff.md) | 10 ms host coverage, pending hardware envelope and proposed controller coordination |
| [Barr-C review](BARR_COMPLIANCE.md) | Coding-standard cleanup, approved compatibility exceptions and verification limits |

The linked guides include historical implementation notes; current code and
dated test evidence determine what is implemented and verified today.

Testing resources remain together under `testing/subsystem3/`:

- [Diagnostic](../../testing/subsystem3/diagnostics/README.md): separate USB bench firmware.
- [Test log](../../testing/subsystem3/TEST_LOG.md): test cases, dated results and evidence references.
- `testing/subsystem3/logs/`: saved evidence from previous runs.

Paths written in backticks in these documents are repository-root paths unless
explicitly stated otherwise.

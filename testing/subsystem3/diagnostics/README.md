# Member 3 hardware diagnostic

`member3_ir_diagnostic.c` is a standalone Pico SDK bench program for inspecting
the three IR sensors through USB serial. It captures black/white references,
reports sensor levels and line observations, and accepts runtime mapping text.
It does not actuate motors or run the integrated micro T-Kernel application.
Its 100 ms logging interval is for stationary inspection, not moving barcode
decoding. Calibration and mapping are RAM-only and do not transfer to the
integrated application.

Build the optional `member3_ir_diagnostic` target from the root CMake project
with `BUILD_MEMBER3_IR_DIAGNOSTIC=ON`. The complete local build and operating
instructions are in [Member 3 guide](../../../docs/buddy3/member3.md#bench-build-and-use).
The target uses the standalone algorithms in `src/line/` and `include/line/`.
The integrated firmware remains under `app_program/subsystem3/` and is built
using `build_make/`.

Loading the diagnostic replaces the firmware on the Pico. Reload the integrated
firmware after bench testing. Record actual test results in
`../TEST_LOG.md` and save evidence in `../logs/`.

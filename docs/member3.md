# Member 3: IR and barcode navigation algorithms

## Implemented

- Digital GPIO snapshot driver: GP2 = left line sensor, GP4 = right line sensor,
  GP27 = barcode sensor. Left/right is the car's perspective facing forward.
- Hardware-independent observations, polarity calibration, normalization, and
  caller-configured temporal filtering. All storage is fixed size.
- Proposed state, operating-mode, command, and event types for Member 3's
  integration boundary. These are types only, not an implemented controller API.
- Separate USB serial bench firmware; existing Member 5 firmware is unchanged.
- Host tests for mixed polarity, insufficient/noisy calibration, invalid input,
  filtering, backwards timestamps, and observation-counter overflow.
- Line observation and correction-direction classification for the selected
  both-black-centred mounting arrangement.
- Code 39 decoder for a single A-Z character, including reversed scans, plus
  a bounded timestamped capture state machine with caller-configured timing.
- Runtime 26-letter action table with validated, transactional text updates.
- Front-pair-only stable-pattern filtering, independent of barcode flicker.
- Distance-qualified junction detector for a transverse black stroke crossing
  the tracked line, using the side sensor and Member 2's forward odometry.

## Bench build and use

With Pico SDK/toolchain configured, configure CMake with
`-DBUILD_MEMBER3_IR_DIAGNOSTIC=ON`, then build target `member3_ir_diagnostic`.
The UF2 is `build/member3_ir_diagnostic.uf2` when using build directory `build`.
Flash that UF2 to run the standalone diagnostic; it replaces the firmware on the
board and does not run the other subsystems. GPIOs are inputs without pulls;
verify the actual sensor DOUT electrical compatibility before connecting.

The build verified on this machine uses its installed SDK and picotool:

```powershell
$env:PICO_SDK_PATH = "$env:USERPROFILE/.pico-sdk/sdk/2.3.1"
$env:PICO_TOOLCHAIN_PATH = "$env:USERPROFILE/.pico-sdk/toolchain/15_2_Rel1"
& "$env:USERPROFILE/.pico-sdk/cmake/v4.3.4/bin/cmake.exe" -S . -B build/member3-pico -G Ninja "-DCMAKE_MAKE_PROGRAM=$env:USERPROFILE/.pico-sdk/ninja/v1.13.2/ninja.exe" "-Dpicotool_DIR=$env:USERPROFILE/.pico-sdk/picotool/2.3.1/picotool" -DBUILD_MEMBER3_IR_DIAGNOSTIC=ON
& "$env:USERPROFILE/.pico-sdk/cmake/v4.3.4/bin/cmake.exe" --build build/member3-pico --target member3_ir_diagnostic -j 4
```

That configuration produces `build/member3-pico/member3_ir_diagnostic.uf2`.
Pointing to installed picotool avoids an unnecessary SDK download.

Open its USB serial output. Readings are logged every 100 ms. This slow interval
is for bench inspection only and is unsuitable for moving barcode capture.
Each CSV line contains timestamp in microseconds, the three raw digital levels,
normalized black mask, calibration valid mask, line observation, and map revision.
Bits 0/1/2 correspond to left / right / barcode. A black mask of -1 means calibration has not
been accepted for all channels.

1. Keep the robot stationary. Put **all three sensors** over a black reference
   surface at the intended mounting height, then send `b`.
2. Keep the surface still until the 20-sample capture completes (about 2 seconds).
3. Put all three sensors over a white reference surface and send `w`.
4. Calibration succeeds only if each sensor gives a stable value throughout both
   captures and those values differ between black and white. On rejection,
   inspect mounting, surface and the sensor's physical sensitivity adjustment,
   then repeat both captures. `r` clears observations and calibration.

This learns digital polarity, not an ADC reflectance threshold. Calibration is
RAM-only and is lost at reset. Rejection means the supplied references cannot
be distinguished reliably; it does not prove a sensor has failed. The strict
reference check avoids inventing a tolerated noise percentage.

The reusable filter requires an explicit stability duration in microseconds;
zero passes samples through. Before initial stability it returns false. After
initial stability it retains the last stable pattern while a new pattern is
pending. It filters the whole three-bit snapshot, so it must **not** be used as
a barcode edge capture mechanism. The bench harness logs raw, unfiltered data
for calibration evidence. No runtime filter duration is selected yet.

## Host verification

From the repository root, with GCC available:

```powershell
New-Item -ItemType Directory -Force build/member3-host
gcc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude src/line/ir_input.c tests/member3_ir_test.c -o build/member3-host/member3_ir_test.exe
& ./build/member3-host/member3_ir_test.exe
gcc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude src/line/ir_input.c src/line/barcode.c src/line/navigation_map.c src/line/line_tracking.c tests/member3_navigation_test.c -o build/member3-host/member3_navigation_test.exe
& ./build/member3-host/member3_navigation_test.exe
gcc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude src/line/ir_input.c src/line/line_tracking.c src/line/junction.c tests/member3_junction_test.c -o build/member3-host/member3_junction_test.exe
& ./build/member3-host/member3_junction_test.exe
```

## Selected line mounting and interpretation

Place the two effective sensing points symmetrically, slightly inside the edges
of the 18 mm black line. Adjust the mounting so a small lateral deviation makes
one sensor see white. At centre both see black; both white means no line is
currently observed. This avoids confusing centred-on-white with a missing line.
If the present mounting straddles the entire line, move the sensors before
using these rules. The optical sensing points matter, not PCB centre positions.

| Left | Right | Observation / requested correction |
| --- | --- | --- |
| Black | Black | Centred / straight |
| Black | White | Correct left |
| White | Black | Correct right |
| White | White | Line absent; request controller handling |

Two binary sensors give a centred band, not a continuous position measurement
or an exact centre. Both black can also occur on a wide junction; the classifier
alone cannot distinguish junctions from ordinary centred tracking. Member
2/controller must set speed and correction strength, confirm line loss/recovery
over time, and suppress tracking corrections during bypass or manoeuvres.
CSV observation values: 0 centred, 1 correct left, 2 correct right, 3 absent.

`line_tracker_update()` adds caller-configured stability time to those rules.
It filters only bits 0/1, so side-sensor barcode flicker cannot delay line updates.
While a new front-pair pattern is settling, the last stable observation is
returned. Startup returns false until a stable pattern is available. The chosen
duration must remain shorter than the tolerable line-loss response time.

## Cross-junction detector

The user's track photo shows a transverse black stroke crossing a longitudinal
black line on a light mat. Normal centred tracking puts both front sensors on
black and the offset sensor on white. At a crossing, the offset sensor crosses
the transverse stroke while the front pair stays on the main line.

`junction_update()` requires normalized sensor inputs and Member 2's cumulative
forward distance in micrometres for every sample. It follows this sequence:

1. Both front sensors black, side white for a measured clearance distance.
2. Side becomes black; record start distance and timestamp.
3. Side returns to white after a stroke within calibrated width bounds and
   minimum duration; both front sensors must have retained the line throughout.
4. Side stays white for a further clearance distance; emit one
   `NAV_EVENT_JUNCTION`. The event does not request a turn automatically.

No event is issued until after exiting the stroke. The controller must account
for the sensor-to-wheel offset and confirmation distance when stopping or
turning. This implementation does not support a turn that must start before
the side sensor exits the crossing.

Ordinary straight lines never activate the side sensor. To distinguish barcode
bars, `min_span_um` must exceed the widest physically measured barcode black
stroke, including optical/odometry uncertainty. Sampling movement must be less
than the smallest measured white barcode gap so adjacent bars cannot merge.
The detector checks those config relationships and subtracts one maximum sample
step from the observed black span when applying the minimum-width threshold.
It rejects oversized patches, lost front-line observations, sample time gaps,
distance jumps, backwards time/distance, and use while not tracking forward.

Required measured configuration:

- `max_barcode_bar_um`, `min_barcode_gap_um`: worst-case effective barcode widths
  at the actual mounting height, angle and sensor sensitivity.
- `min_span_um`, `max_span_um`: effective crossing stroke bounds, with room for
  sample-edge uncertainty. The maximum must accommodate the chosen sample step.
- `clear_span_um`: sustained side-white clearance before/after a stroke.
- `max_sample_step_um`, `max_sample_gap_us`: permitted sampling movement/time.
- `min_black_us`, `max_crossing_us`: glitch rejection and crossing timeout.

No unmeasured hardware defaults are set. The test's 9 mm barcode / 18 mm crossing
geometry is synthetic validation only. If barcode and crossing widths overlap
under real operating conditions, this evidence is insufficient: do not lower
thresholds to force a result. Additional sensing or a controller-managed probe
would be required. Likewise, a black patch with the same stroke width can look
like a junction. This detects a transverse junction candidate under the track
layout assumptions; it does not prove four-way topology or detect a T branch
which exists only on the opposite side from the third sensor.

The detector uses fixed storage and performs no GPIO, motor or encoder control.
Member 3's owning task must pass calibrated samples, reliable forward odometry,
and `tracking_forward=false` during pause, turning, recovery, and bypass. Queue
its detected event to the Mission Controller. That team integration is pending;
the standalone 100 ms USB diagnostic does not have odometry and therefore does
not run junction detection.

## Testing the pictured black line on the actual mat

Calibrate using the actual black tape/ink and light mat at the installed sensing
height, rather than assuming visible colours guarantee infrared contrast. Adjust
each sensor's physical sensitivity until black and white produce opposite stable
DOUT readings. If they cannot, software polarity calibration will reject them;
software cannot manufacture reflectance contrast.

With both front sensing spots inside the line edges, verify these stationary
positions in the USB CSV: centre gives both black, moving the car right leaves
only the left sensor black, moving left leaves only the right black, and moving
fully off the line gives both white. The black mask's lowest two bits must read
3, 1, 2, 0 respectively, irrespective of raw GPIO polarity. Pass over the pictured
junction while logging all three channels and synchronized encoder distance.
Use those measurements to set the junction bounds, then repeat at each operating
speed and expected lighting, and over every barcode size. Verify exactly one
junction event per crossing and none during normal line/barcode passages.

## Barcode decoding and capture

The supplied Barcode Sample PDF contains 29 alternating runs matching `*A*`
and `*Z*` at two sizes. Their nine-element symbols match the Code 39 start/stop
asterisk and A/Z assignments. Encoding reference: [USS-39 Table 2](https://expresscorp.com/wp-content/uploads/2023/02/USS-39.pdf).
All A-Z assignments are tested using independent bar/space reference patterns;
actual raster widths from the supplied PDF are also tested.

`barcode_decode()` accepts exactly 29 black/white widths for `*LETTER*`, checks
start/stop and width separation, and retries the reversed run sequence.
It rejects unsupported characters, partial frames and invalid widths. This is
a single-letter course decoder, not a general Code 39 text/checksum decoder.
Physical print size and scan speed may scale widths; rapid changes of speed or
reversing halfway through one barcode are not supported.

`barcode_capture_update()` takes normalized side-sensor levels and monotonic
microsecond timestamps. It requires initial white quiet time, collects each
transition into fixed storage, finishes after trailing white quiet time, and
emits one result per traversal. Timeout/overflow/invalid patterns produce no
valid letter. Quiet-zone duration and frame timeout are explicit initialization
arguments, not invented runtime defaults. Sampling must resolve the narrowest
bar at the maximum operating speed, or timestamped GPIO edges must be provided.
Do not apply the whole-three-channel IR filter to barcode transitions.
The 100 ms diagnostic harness does not run live barcode capture: it is too slow.

## Runtime command remapping

Defaults requested by the user: A=FORWARD, B=LEFT, C=RIGHT, D=UTURN.
E-Z default to NONE (decoded but no action). UTURN means a 180-degree navigation
turn, not reverse driving. The available motion actions remain the four
specified by the professor; NONE disables a letter. Decode direction does not
change the character or its action.

Example text update, also accepted by USB diagnostic firmware with Enter:

```text
MAP A=FORWARD Z=UTURN W=LEFT F=RIGHT B=NONE C=NONE D=NONE
```

This changes the listed entries only. Unlisted entries retain their previous
actions; disable former assignments explicitly with NONE if desired. Multiple
letters may share an action. Uppercase A-Z and exact action names are required;
duplicate letters, empty updates, malformed entries and unknown actions reject
the entire update without changing the map. Each accepted update increments a
revision number. Changes take effect without recompilation and are RAM-only;
reset restores defaults. No flash persistence was requested.

Member 1 receives Wi-Fi/MQTT payloads and passes a bounded, NUL-terminated text
update to the owning task; it must not write this table from a network callback
while another task reads it. `nav_map_apply()` and `nav_map_lookup()` are called
serially by the owner (or protected by the team's actual RTOS synchronization).
Return true/false plus revision as the configuration acknowledgement. Transport,
topic name, JSON envelope and authentication belong to Member 1, not this module.
The USB adapter accepts at most 511 characters and rejects overflow.

After a valid letter, look up the current action and send a validated event to
the Mission Controller. An unmapped letter issues no movement request. Once
the Mission Controller accepts a manoeuvre, later mapping changes must not
alter that manoeuvre; resolve/snapshot the action when creating the event.
Those task/queue connections are pending the team's interface.

## Still deferred until supplied or measured

- Steering gain/speed and connections to the real motor/controller interfaces.
- Actual junction/line calibration, odometry delivery and controller queue hookup.
- Hardware validation of line-loss/recovery timing and reacquisition confirmation.
- Final capture quiet/timeout timing, sensor filtering, width tolerance tuning
  and hardware validation across intended speeds.
- Actual navigation callbacks/queues, Mission Controller contracts, manoeuvre
  acknowledgement and telemetry transport.
- Final team priority allocation and on-board RTOS scheduling validation.
- Exact Barr C conformance audit against the course's supplied standard.

The state/event enums reserve the confirmed responsibilities, including U-turn;
the junction algorithm returns an event structure, but transport to the actual
controller is not wired and none of these algorithms drives the robot.
The separate `app_program/subsystem3/` template integration now provides the
owner task, message buffers, navigation state machine and recovery handling.
See [template integration](member3-template-integration.md) for the runtime API,
installation/build instructions, and verified scope. The original CMake
diagnostic remains a standalone bench tool.
Member 2 owns motors/encoders; Member 1 owns MQTT; Members 4/5 own IMU/obstacles.
Member 3 will report line reacquisition through the Mission Controller rather
than implementing Member 5's avoidance/recovery manoeuvres.

## Verification status

IR, navigation and junction host tests passed with C11 and strict warnings, including all
26 letters at multiple scales in both directions, timestamp capture, supplied
A/Z PDF patterns, transactional mapping failures, and all line input patterns.
Junction tests cover an 18 mm cross stroke at different simulated speeds,
straight lines, barcode strokes/trains, noise, oversized patches, partial-start
scans, line loss, disabled tracking, stationary sensors, bad odometry/timestamps,
invalid configuration, and side-independent front-pair filtering.
Pico W diagnostic firmware and the reusable navigation library
compiled and linked with the installed Pico SDK 2.3.1 and ARM GCC toolchain;
UF2 generation succeeded. No board was flashed and no physical sensor tests
were performed. These checks validate the software foundation, not line
tracking, barcode recognition, electrical wiring, or course performance.

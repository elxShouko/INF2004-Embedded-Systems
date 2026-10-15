# Reference traceability and conflicts

Sources were read from `reference_docs/`, not inferred from old branch code.

| Authority | Implementation / verification | Open limitation |
|---|---|---|
| Project Write-up pp. 1–3, 8; Briefing p. 7 | C/Pico SDK/micro T-Kernel; IMU sensing, calibration, tilt, motion, collision, hump/peak reporting | Height accuracy and vehicle integration require physical evidence |
| Actual main template, README, Make source discovery | `app_program/subsystem4`, init then start; approved SDK I2C/math additions; four Make profiles | No CMake application or RTOS replacement |
| Week 6 FR7–FR9, NFR5, sections 4/6.4 | Calibration → monitoring → ascent → peak → descent → monitoring; independent turning events; peak-time maximum | Timing, thresholds, height formula and filtering were not specified as calibrated values |
| Week 6 section 6.4 versus system FAULT/reset rules in section 5 | Latched faults; no automatic resumption | Controller reset/stop interfaces still need agreement |
| Week 6 A-B4-1/A-B4-2 and section 8.2 | Explicit unvalidated settings and manual calibration test cases | Host fixtures are not measured calibration |
| Pico W and Robo Pico datasheets under hardware/ | GP16/GP17 design assignment; 3.3 V-compatible bus verification required | Breakout identity, pull-ups, mounting and physical wiring unverified |
| Barr-C:2018 sections 1–8 | Fixed-width values, float32_t, static internals, module APIs, Allman braces, bounded state, explicit errors | See BARR_REVIEW.md for scope and inherited exceptions |
| TEAM.md and subsystem integration code | Retained S3 summary buffer/flags, monitor demo and ownership boundaries | No new agreed Mission Controller or MQTT message layout invented |
| testing/README.md; videos/README.md | Stable IDs, append-only results, source hashes, uncommitted revision disclosure | No on-target or physical PASS recorded |

The hardware folder does not contain the sensor register datasheet. The
supplementary primary source is [ST LSM303DLHC, DocID018771 Rev 2](https://www.st.com/resource/en/datasheet/lsm303dlhc.pdf),
sections 5–7 and table 3: register addresses, ID bytes, byte/axis ordering,
BDU, sensitivities and supported rates. The part is an accelerometer plus
magnetometer, not a gyro. The installed Pico SDK `hardware_i2c/i2c.c` and
`pico_time/timeout_helper.c` were also inspected rather than assuming its
timeout APIs eliminate every internal busy wait.

Confirmed versus proposed: the academic requirement is hump-height estimation
and highest-hump reporting. The references do not prescribe its formula.
The implemented distance/tilt integral is a guarded development proposal,
described with its chassis-geometry limitations in README.md. No fabricated
wheelbase, speed calibration or slope-as-height conversion is used. Buddy 2's
speed provider and Buddy 1's telemetry units/invalidity encoding still need
agreement. No callers or teammate code were changed to imply that agreement.

The user explicitly approved the shared Make/TEAM changes and chose Week 6's
peak-time reporting instead of the archived branch's completed-descent update.
Fault recovery follows the conservative system-level reset rule; recovery of
the sensor alone must never imply permission for the vehicle to move.


# Buddy 4 reference review and unresolved integration work

Historical inspection of the archived implementation. Current implementation
traceability and resolved/open items are in
[`testing/buddy4/REFERENCE_REVIEW.md`](../buddy4/REFERENCE_REVIEW.md).

## Authoritative requirements

The supplied Project Write-up, page 3, requires an IMU-based hump-height
estimate. Page 8 assigns Buddy 4 IMU integration, calibration, tilt/hump/peak
measurement, motion classification, collision detection, turn-rate monitoring
and event reporting. The supplied Project Briefing, page 7, repeats these
responsibilities. A pitch peak alone must not be presented as hump height.

Week_6_Design_Review.docx FR7–FR9 and NFR5 cover acquisition, tilt/motion state,
hump detection and highest-peak retention, turning, impacts, and repeatability.
Sections 2/3/4 assign the LSM303DLHC on GP16/GP17 to Buddy 4 and send terrain
events to the Mission Controller. Section 8.2 reserves motor actuation for
Buddy 2. A-B4-1 and A-B4-2 require calibration and repeated physical measurements;
§8.2 explicitly defers thresholds, sampling and filtering to those measurements.

The local hardware folder has the Pico W and Robo Pico datasheets, but no
LSM303DLHC datasheet. The supplementary manufacturer source is
[ST DS7202](https://www.st.com/resource/en/datasheet/lsm303dlhc.pdf).
It describes an accelerometer and magnetometer, without a gyroscope. Heading
differences may estimate turn rate; tests of that estimate do not validate
angular-rate accuracy on the moving car. The local Robo Pico datasheet specifies
digital input high up to 3.6 V and analog input up to 3.3 V; board power and
sensor logic voltage are different quantities. Physical wiring remains unverified.

## Findings from the preserved implementation

These are repository findings, not new agreed interfaces or calibrated values.
The host runner preserves them for regression testing; it does not enable them
in the integrated firmware.

| Finding | Consequence / required action |
|---|---|
| Current `app_program/subsystem4` is the monitor demonstration. | Host results for archived `src/imu` cannot be labelled current-firmware results. |
| `imu_rtos_start` creates objects and starts its task in one call. | Split creation into subsystem init and task startup into subsystem start, preserving the all-init-before-any-start rule. |
| Archived `g_b_running`/`g_b_stop` are `volatile`; some `g_stats` writes occur outside the reader's mutex. | Correct cross-core synchronisation before SMP use. A single-thread fake kernel cannot detect these races. |
| The wrapper defaults to priority 9; current TEAM.md reserves 18–19 for S4. | Resolve priorities with owners; no implicit priority change. |
| Old hardware adapter uses Pico SDK `hardware_i2c` and math functions. | Review Make dependency/link requirements; the template does not link the complete Pico SDK runtime automatically. |
| The template I2C driver is disabled; its configured GP8/GP9 conflict with Buddy 2. | Verify peripheral ownership and GP16/GP17 wiring before activating I2C0. |
| Legacy `hump_peak` is centidegrees; Buddy 1 currently has `hump_peak_x10`. | Agree meaning, units, conversion, invalid-value handling, and measurement validity with Buddy 1. |
| Ranking defaults to peak pitch, with a geometric height estimate and placeholder wheelbase. | Confirm the physical height method and ranking criterion against known hump shapes; slope alone cannot rank arbitrary hump heights. |
| The old detector commits the maximum after a validated descent; DR §6.4 says compare/update at peak. | Review provisional-versus-validated peak semantics with the team. Do not silently call this agreed. |
| DR §6.4 permits sensor recovery to monitoring; system §5 requires reset after FAULT. | Define sensor recovery separately from permission for the vehicle to resume driving. |
| Extra legacy event types and encoder hints have no agreed shared transport. | Agree message ownership, units, lifetime, overflow, and mission reset behavior before integration. |
| Critical notification depends on configured IDs and ignores a flag-set failure. | Define and validate the bounded fault/stop path; a host event capture is not a safe-stop implementation. |

## Required physical evidence

Before field acceptance, record sensor identity and breakout supply/pull-ups,
mounting orientation, axis map, calibration data, vehicle geometry, sensor rates,
actual task timing/jitter, filters, thresholds, and build revision.

Perform stationary/noise and controlled tilt measurements; drive straight and
turn at measured speeds; repeat known humps at controlled speeds; include humps
with differing lengths/slopes; and compare estimated versus measured height.
Report bias, spread, missed/false events, and whether the highest actual hump is
selected. Agree numerical acceptance limits before these trials.

Test communication failure and bounded event handling under realistic load,
including a full normal event buffer. Use a separately authorised restrained
impact test and Mission Controller/Buddy 2 stop-path test. Save evidence under
`logs/` and use `videos/README.md` for the required video index. No physical tests
or on-target tests were performed by this test-folder update.

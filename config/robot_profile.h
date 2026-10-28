/** @file robot_profile.h @brief Measured settings; rebuild/reboot to apply. */
#ifndef ROBOT_PROFILE_H
#define ROBOT_PROFILE_H
/* Confirmed wiring: motors 8/9,10/11; encoders 7/28,0/1; IR 2,4,27;
 * IMU 16/17; ultrasonic TRIG26/ECHO6; one servo GP15.
 * ECHO is professor-approved, not electrically measured. GP28 jumper is
 * physically unverified; professor instructed ignoring it. No ADC/VBAT use.
 * This records wiring, not calibrated scale, direction or power authority.
 *
 * Every omitted member is ZERO/UNKNOWN, deliberately. Do not paste test
 * fixture values. See docs/commissioning.md and the production type headers.
 */
static vehicle_robot_profile_t const g_vehicle_profile =
{
    .stage = VEHICLE_STAGE_BENCH,
    .run_limit_ms = 0U,
    .bench =
    {
        .motor =
        {
            .clock_hz = 125000000U /* Platform clock, not calibration. */
        },
        .b_encoder_on_boot = true /* Raw count diagnostics; NO motor enable. */
    },
    .setup =
    {
        .obstacle =
        {
            .acquisition =
            {
                .servo_gpio = 15U,
                .b_wiring_confirmed = true
            }
        }
    }
};
#endif /* ROBOT_PROFILE_H */

/*** end of file ***/

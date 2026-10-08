#include "obstacle/ultrasonic.h"

#include <stdbool.h>

#include "obstacle/obstacle_config.h"
#include "pico/stdlib.h"

#define SOUND_SPEED_CM_PER_US 0.0343f

static bool wait_for_echo_level(
    bool level,
    uint64_t timeout_us)
{
    uint64_t start_time_us;

    start_time_us = time_us_64();

    while (gpio_get(OBSTACLE_ECHO_GPIO) != level)
    {
        if ((time_us_64() - start_time_us) >= timeout_us)
        {
            return false;
        }

        tight_loop_contents();
    }

    return true;
}

void ultrasonic_init(void)
{
    gpio_init(OBSTACLE_TRIG_GPIO);
    gpio_set_dir(OBSTACLE_TRIG_GPIO, GPIO_OUT);
    gpio_put(OBSTACLE_TRIG_GPIO, 0);

    gpio_init(OBSTACLE_ECHO_GPIO);
    gpio_set_dir(OBSTACLE_ECHO_GPIO, GPIO_IN);
}

ultrasonic_status_t ultrasonic_read_distance_cm(
    uint32_t *distance_cm)
{
    uint64_t echo_start_us;
    uint64_t echo_end_us;
    uint64_t pulse_width_us;
    float calculated_distance_cm;

    if (distance_cm == NULL)
    {
        return ULTRASONIC_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Ensure ECHO is low before starting another measurement.
     */
    if (!wait_for_echo_level(
            false,
            OBSTACLE_ECHO_TIMEOUT_US))
    {
        return ULTRASONIC_STATUS_TIMEOUT_FALL;
    }

    /*
     * HC-SR04 trigger pulse.
     */
    gpio_put(OBSTACLE_TRIG_GPIO, 0);
    sleep_us(2);

    gpio_put(OBSTACLE_TRIG_GPIO, 1);
    sleep_us(10);
    gpio_put(OBSTACLE_TRIG_GPIO, 0);

    /*
     * Wait for ECHO rising edge.
     */
    if (!wait_for_echo_level(
            true,
            OBSTACLE_ECHO_TIMEOUT_US))
    {
        return ULTRASONIC_STATUS_TIMEOUT_RISE;
    }

    echo_start_us = time_us_64();

    /*
     * Wait for ECHO falling edge.
     */
    if (!wait_for_echo_level(
            false,
            OBSTACLE_ECHO_TIMEOUT_US))
    {
        return ULTRASONIC_STATUS_TIMEOUT_FALL;
    }

    echo_end_us = time_us_64();

    pulse_width_us =
        echo_end_us - echo_start_us;

    calculated_distance_cm =
        ((float)pulse_width_us *
         SOUND_SPEED_CM_PER_US) /
        2.0f;

    if ((calculated_distance_cm <
         (float)OBSTACLE_MIN_DISTANCE_CM) ||
        (calculated_distance_cm >
         (float)OBSTACLE_MAX_DISTANCE_CM))
    {
        return ULTRASONIC_STATUS_OUT_OF_RANGE;
    }

    *distance_cm =
        (uint32_t)calculated_distance_cm;

    return ULTRASONIC_STATUS_OK;
}

ultrasonic_status_t ultrasonic_get_validated_distance_cm(
    uint32_t *distance_cm)
{
    uint32_t samples[OBSTACLE_VALIDATION_SAMPLES];
    uint32_t minimum;
    uint32_t maximum;
    uint32_t total;
    ultrasonic_status_t status;

    if (distance_cm == NULL)
    {
        return ULTRASONIC_STATUS_INVALID_ARGUMENT;
    }

    total = 0U;

    for (uint32_t index = 0U;
         index < OBSTACLE_VALIDATION_SAMPLES;
         index++)
    {
        status =
            ultrasonic_read_distance_cm(
                &samples[index]);

        if (status != ULTRASONIC_STATUS_OK)
        {
            return status;
        }

        total += samples[index];

        if ((index + 1U) <
            OBSTACLE_VALIDATION_SAMPLES)
        {
            sleep_ms(60);
        }
    }

    minimum = samples[0];
    maximum = samples[0];

    for (uint32_t index = 1U;
         index < OBSTACLE_VALIDATION_SAMPLES;
         index++)
    {
        if (samples[index] < minimum)
        {
            minimum = samples[index];
        }

        if (samples[index] > maximum)
        {
            maximum = samples[index];
        }
    }

    if ((maximum - minimum) >
        OBSTACLE_VALIDATION_TOLERANCE_CM)
    {
        return ULTRASONIC_STATUS_INCONSISTENT;
    }

    *distance_cm =
        total / OBSTACLE_VALIDATION_SAMPLES;

    return ULTRASONIC_STATUS_OK;
}

const char *ultrasonic_status_name(
    ultrasonic_status_t status)
{
    const char *name;

    switch (status)
    {
        case ULTRASONIC_STATUS_OK:
            name = "OK";
            break;

        case ULTRASONIC_STATUS_INVALID_ARGUMENT:
            name = "INVALID_ARGUMENT";
            break;

        case ULTRASONIC_STATUS_TIMEOUT_RISE:
            name = "TIMEOUT_RISE";
            break;

        case ULTRASONIC_STATUS_TIMEOUT_FALL:
            name = "TIMEOUT_FALL";
            break;

        case ULTRASONIC_STATUS_OUT_OF_RANGE:
            name = "OUT_OF_RANGE";
            break;

        case ULTRASONIC_STATUS_INCONSISTENT:
            name = "INCONSISTENT";
            break;

        default:
            name = "UNKNOWN";
            break;
    }

    return name;
}
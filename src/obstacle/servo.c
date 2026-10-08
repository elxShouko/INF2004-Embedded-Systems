#include "obstacle/servo.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "obstacle/obstacle_config.h"

#define SERVO_MAX_GPIO 28U
#define SERVO_MAX_ANGLE_DEG 180U

static uint32_t servo_gpio_pin = 255U;
static uint32_t servo_slice = 0U;
static uint16_t servo_wrap = 65535U;

static uint8_t current_angle_deg =
    OBSTACLE_SCAN_CENTER_DEG;

static bool ready = false;

bool servo_init(uint32_t gpio_pin)
{
    uint32_t system_clock_hz;
    float clock_divider;

    if (gpio_pin > SERVO_MAX_GPIO)
    {
        ready = false;
        return false;
    }

    servo_gpio_pin = gpio_pin;

    gpio_set_function(
        servo_gpio_pin,
        GPIO_FUNC_PWM);

    servo_slice =
        pwm_gpio_to_slice_num(
            servo_gpio_pin);

    system_clock_hz =
        clock_get_hz(clk_sys);

    clock_divider =
        (float)system_clock_hz /
        ((float)OBSTACLE_SERVO_FREQUENCY_HZ *
         ((float)servo_wrap + 1.0f));

    pwm_set_clkdiv(
        servo_slice,
        clock_divider);

    pwm_set_wrap(
        servo_slice,
        servo_wrap);

    pwm_set_enabled(
        servo_slice,
        true);

    ready = true;

    return servo_set_angle(
        OBSTACLE_SCAN_CENTER_DEG);
}

bool servo_set_angle(uint8_t angle_deg)
{
    uint32_t pulse_width_us;
    uint32_t pwm_level;

    if (!ready)
    {
        return false;
    }

    if (angle_deg > SERVO_MAX_ANGLE_DEG)
    {
        return false;
    }

    pulse_width_us =
        OBSTACLE_SERVO_MIN_PULSE_US +
        (((uint32_t)angle_deg *
          (OBSTACLE_SERVO_MAX_PULSE_US -
           OBSTACLE_SERVO_MIN_PULSE_US)) /
         SERVO_MAX_ANGLE_DEG);

    pwm_level =
        (((uint32_t)servo_wrap + 1U) *
         pulse_width_us) /
        OBSTACLE_SERVO_PERIOD_US;

    pwm_set_gpio_level(
        servo_gpio_pin,
        (uint16_t)pwm_level);

    current_angle_deg = angle_deg;

    return true;
}

uint8_t servo_get_angle(void)
{
    return current_angle_deg;
}

bool servo_is_ready(void)
{
    return ready;
}
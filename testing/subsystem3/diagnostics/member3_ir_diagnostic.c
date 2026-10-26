#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "line/ir_gpio.h"
#include "line/line_tracking.h"
#include "line/navigation_map.h"

/* Bench logging only; these are NOT navigation timing/calibration values. */
#define DIAGNOSTIC_PERIOD_MS 100U
#define DIAGNOSTIC_CAPTURE_SAMPLES 20U

static const char *action_name(nav_command_t action)
{
    switch (action)
    {
        case NAV_COMMAND_STRAIGHT: return "FORWARD";
        case NAV_COMMAND_LEFT: return "LEFT";
        case NAV_COMMAND_RIGHT: return "RIGHT";
        case NAV_COMMAND_U_TURN: return "UTURN";
        default: return "NONE";
    }
}

/* USB bench adapter for the same update API Member 1 can call over Wi-Fi.
 * Bounded input, no blocking readline; CSV logging continues while typing.
 */
static int read_command(nav_map_t *p_map)
{
    static char buffer[512];
    static size_t length = 0U;
    static bool collecting = false;
    static bool overflow = false;
    int simple_command = PICO_ERROR_TIMEOUT;

    for (unsigned int count = 0U; count < 128U; ++count)
    {
        const int input = getchar_timeout_us(0U);
        if (input == PICO_ERROR_TIMEOUT)
        {
            break;
        }
        if (!collecting)
        {
            if (input == 'M')
            {
                collecting = true;
                overflow = false;
                length = 0U;
                buffer[length++] = 'M';
            }
            else if ((input == 'b') || (input == 'w') || (input == 'r'))
            {
                simple_command = input;
            }
        }
        else if ((input == '\n') || (input == '\r'))
        {
            buffer[length] = '\0';
            const bool accepted = !overflow && nav_map_apply(p_map, buffer);
            printf("Mapping %s; revision=%lu\n", accepted ? "accepted" : "rejected",
                   (unsigned long)p_map->revision);
            if (accepted)
            {
                for (unsigned int index = 0U; index < NAV_ALPHABET_SIZE; ++index)
                {
                    printf("%c=%s%c", (char)('A' + index),
                           action_name(p_map->actions[index]),
                           (index == NAV_ALPHABET_SIZE - 1U) ? '\n' : ' ');
                }
            }
            collecting = false;
        }
        else if (length < sizeof(buffer) - 1U)
        {
            buffer[length++] = (char)input;
        }
        else
        {
            overflow = true;
        }
    }
    return simple_command;
}

int main(void)
{
    ir_observations_t black = {0};
    ir_observations_t white = {0};
    ir_calibration_t calibration = {0};
    char capture = '\0';
    nav_map_t map;

    nav_map_init(&map);

    stdio_init_all();
    ir_gpio_init();
    sleep_ms(2000U);
    printf("Member 3 IR bench diagnostic; no motor outputs.\n");
    printf("b: capture ALL sensors on black; w: ALL on white; r: reset\n");
    printf("Each capture takes 20 samples (about 2 seconds).\n");
    printf("Runtime mapping: MAP A=FORWARD Z=UTURN W=LEFT F=RIGHT then Enter\n");
    printf("Startup: all letters NONE; supply a runtime mapping.\n");
    printf("timestamp_us,raw_left,raw_right,raw_barcode,black_mask,valid_mask,line_observation,map_revision\n");

    while (true)
    {
        const int command = read_command(&map);
        if ((command == 'b') || (command == 'w'))
        {
            capture = (char)command;
            memset((capture == 'b') ? &black : &white, 0, sizeof(black));
            memset(&calibration, 0, sizeof(calibration));
            printf("Capture %c started; keep ALL sensors on that surface.\n", capture);
        }
        else if (command == 'r')
        {
            memset(&black, 0, sizeof(black));
            memset(&white, 0, sizeof(white));
            memset(&calibration, 0, sizeof(calibration));
            capture = '\0';
        }

        ir_sample_t sample;
        if (ir_gpio_read(&sample))
        {
            if (capture != '\0')
            {
                ir_observations_t *p_capture = (capture == 'b') ? &black : &white;
                (void)ir_observe(p_capture, &sample);
                if (p_capture->samples >= DIAGNOSTIC_CAPTURE_SAMPLES)
                {
                    printf("Capture %c complete.\n", capture);
                    capture = '\0';
                    if ((black.samples >= DIAGNOSTIC_CAPTURE_SAMPLES) &&
                        (white.samples >= DIAGNOSTIC_CAPTURE_SAMPLES))
                    {
                        const bool valid = ir_calibrate(&black, &white,
                            DIAGNOSTIC_CAPTURE_SAMPLES, &calibration);
                        printf("Calibration %s; valid_mask=%u, black_high_mask=%u\n",
                            valid ? "accepted" : "rejected",
                            calibration.valid_mask, calibration.black_high_mask);
                    }
                }
            }
            uint8_t black_mask = 0U;
            const bool normalized = ir_normalize(&sample, &calibration, &black_mask);
            line_observation_t observation = LINE_OBSERVATION_ABSENT;
            if (normalized)
            {
                (void)line_classify(black_mask, &observation);
            }
            printf("%llu,%u,%u,%u,%d,%u,%d,%lu\n",
                (unsigned long long)sample.timestamp_us,
                (sample.levels >> IR_CHANNEL_LINE_1) & 1U,
                (sample.levels >> IR_CHANNEL_LINE_2) & 1U,
                (sample.levels >> IR_CHANNEL_BARCODE) & 1U,
                normalized ? (int)black_mask : -1,
                calibration.valid_mask,
                normalized ? (int)observation : -1,
                (unsigned long)map.revision);
        }
        sleep_ms(DIAGNOSTIC_PERIOD_MS);
    }
}

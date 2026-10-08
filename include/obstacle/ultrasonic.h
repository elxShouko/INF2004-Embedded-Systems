#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

typedef enum
{
    ULTRASONIC_STATUS_OK = 0,
    ULTRASONIC_STATUS_INVALID_ARGUMENT,
    ULTRASONIC_STATUS_TIMEOUT_RISE,
    ULTRASONIC_STATUS_TIMEOUT_FALL,
    ULTRASONIC_STATUS_OUT_OF_RANGE,
    ULTRASONIC_STATUS_INCONSISTENT
} ultrasonic_status_t;

void ultrasonic_init(void);

ultrasonic_status_t ultrasonic_read_distance_cm(
    uint32_t *distance_cm);

ultrasonic_status_t ultrasonic_get_validated_distance_cm(
    uint32_t *distance_cm);

const char *ultrasonic_status_name(
    ultrasonic_status_t status);

#endif
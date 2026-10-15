/**
 * @file motion_foundation_test.h
 * @brief Non-actuating M1B-M1D tests; report failure count to the host runner.
 */
#ifndef MOTION_FOUNDATION_TEST_H
#define MOTION_FOUNDATION_TEST_H

#include <stdint.h>

/* Defined only for APP_TEST or MOTION_CONFIG_HOST_TEST. No hardware access. */
uint32_t motion_foundation_test(void);

#endif /* MOTION_FOUNDATION_TEST_H */

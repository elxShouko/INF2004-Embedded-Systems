/** @file test_drive.h @brief Pure M3-M7 non-actuating tests. */
#ifndef TEST_DRIVE_H
#define TEST_DRIVE_H
#include <stdint.h>
#include <stdbool.h>
/* Native: run existing pure groups on worker too, preserving every assertion.
 * Host: caller already runs those groups; pass NULL for the new suite only.
 */
uint32_t test_drive (bool (*p_config_tests)(void));
#endif /* TEST_DRIVE_H */

/*** end of file ***/


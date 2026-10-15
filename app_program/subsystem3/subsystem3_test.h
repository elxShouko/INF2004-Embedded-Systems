/** @file subsystem3_test.h
 * @brief Declare the retained template's Subsystem 3 entry point.
 * @note S3-DEV-01 preserves required template names and data interfaces.
 */
#ifndef SUBSYSTEM3_TEST_H
#define SUBSYSTEM3_TEST_H

#include "subsystem3.h"

#if APP_TEST
/** @brief Run the professor-provided demonstration checks on target. */
IMPORT void subsystem3_test (void);
#endif

#endif

/*** End of file. ***/


/** @file test_ports.h @brief Host-only boundary fakes; reuse S2 ABI doubles. */
#ifndef TEST_PORTS_H
#define TEST_PORTS_H
#if !defined(VEHICLE_HOST_TEST) || defined(_PICO_RP2040_)
#error "Integration fakes are host only"
#endif
#include <stdbool.h>
#define APP_H
#include "../subsystem2/test_rtos.h"
typedef uint8_t UB;
typedef int32_t PRI;
typedef struct { UW spver; UW prver; } T_RVER;
#define TASK_STACK_SIZE (2048U)
#define TP_PRC1 (1U)
#define TA_WMUL (8U)
#define E_PAR (-17)
#define E_NOSPT (-9)
#define CHECK(error, label) ((void)(error), (void)(label))
ER tk_dly_tsk (TMO delay);
ER tk_slp_tsk (TMO delay);
ER tk_ref_ver (T_RVER * p_version);
INT tk_get_prc (void);
void tm_printf (UB * p_format, ...);
void subsystem1_test (void);
void subsystem2_test (void);
void subsystem3_test (void);
void subsystem4_test (void);
void subsystem5_test (void);
ER subsystem1_init (void);
ER subsystem3_init (void);
ER subsystem4_init (void);
ER subsystem5_init (void);
ER subsystem1_start (void);
ER subsystem3_start (void);
ER subsystem4_start (void);
ER subsystem5_start (void);
INT usermain (void);
uint32_t test_port_read (uint32_t address);
void test_port_write (uint32_t address, uint32_t value);
uint32_t test_port_core (void);
bool test_port_enter (uint32_t * p_saved);
void test_port_leave (uint32_t saved);
bool test_port_install (void (*p_handler)(UINT number));
#endif
/*** end of file ***/

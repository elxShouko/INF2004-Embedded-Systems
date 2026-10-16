/** @file test_s5_port.h @brief Host-only MMIO boundary for real S5 driver. */
#ifndef TEST_S5_PORT_H
#define TEST_S5_PORT_H
#if defined(__arm__) || defined(__thumb__)
#error "MMIO fakes must never target the physical RP2040"
#endif
#ifndef S5_DRIVER_HOST_TEST
#error "Host fake requires explicit test selection"
#endif
#include <stdint.h>
#include <stddef.h>
#include "sys/sysdepend/cpu/rp2040/sysdef.h"
typedef uint32_t UINT;
typedef int32_t ER;
#define E_OK (0)
#define E_SYS (-5)
#define GPIO_MODE_IN (0U)
#define GPIO_MODE_OUT (1U)
#define DEVCONF_ADC_PIN_INIT_0 (0)
#define DI(saved) ((saved) = test_s5_enter())
#define EI(saved) test_s5_leave(saved)
uint32_t test_s5_enter (void);
void test_s5_leave (uint32_t saved);
uint32_t in_w (uint32_t address);
void out_w (uint32_t address,uint32_t value);
void clr_w (uint32_t address,uint32_t value);
ER gpio_set_pin (UINT pin,UINT mode);
ER pwm_set_pin (UINT pin);
void WaitUsec (uint32_t delay);
#endif
/*** end of file ***/

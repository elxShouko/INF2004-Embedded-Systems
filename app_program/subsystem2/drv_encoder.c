/** @file drv_encoder.c
 * @brief Encoder-only GPIO setup; shared interrupt ownership stays external.
 */
#include "drv_encoder.h"
#if defined(_PICO_RP2040_) && !defined(MOTION_CONFIG_HOST_TEST)
#include <stddef.h>
#define PROHIBIT_DEF_SIZE_T 1
#include <tk/tkernel.h>
#include <tk/syslib.h>
#include <bsp/libbsp.h>
#include <config_device.h>
#include "../../device/adc/sysdepend/rp2040/adc_cnf_sysdep.h"

bool drv_encoder_open (drv_encoder_port_t const * p_port,
    drv_encoder_sink_t p_sink, void * p_context)
{
    bool b_ok = (NULL != p_port) && (NULL != p_sink);
    static uint32_t const pins[4] = {7U, 28U, 0U, 1U};

    if (b_ok)
    {
        b_ok = (NULL != p_port->p_reserve) &&
            (NULL != p_port->p_enable) && (NULL != p_port->p_release) &&
            (0 == DEVCNF_USE_IIC) && (0 == DEVCONF_ADC_PIN_INIT_2) &&
            (0U == (in_w(GPIO_OE) & ENC_CAPTURE_MASK));
#if !BOARD_UART0_FREE || TM_CONSOLE_UART
        b_ok = false;
#endif
    }
    for (uint32_t index = 0U; b_ok && (index < 4U); ++index)
    {
        uint32_t control = in_w(GPIO_CTRL(pins[index]));
        uint32_t function = control & 0x1FU;

        b_ok = ((5U == function) || (31U == function)) &&
               (0U == (control & 0x30033300U));
    }
    if (b_ok)
    {
        b_ok = p_port->p_reserve(p_port->p_context, ENC_CAPTURE_MASK,
                                 p_sink, p_context);
        if (b_ok)
        {
            for (uint32_t index = 0U; index < 4U; ++index)
            {
                if (E_OK != gpio_set_pin(pins[index], GPIO_MODE_IN))
                {
                    b_ok = false;
                }
            }
            if (b_ok)
            {
                p_sink(p_context, ENC_CAPTURE_MASK, in_w(GPIO_IN));
                b_ok = p_port->p_enable(p_port->p_context, ENC_CAPTURE_MASK);
            }
            if (!b_ok)
            {
                p_port->p_release(p_port->p_context, ENC_CAPTURE_MASK);
            }
        }
    }

    return (b_ok);
}

void drv_encoder_close (drv_encoder_port_t const * p_port)
{
    if ((NULL != p_port) && (NULL != p_port->p_release))
    {
        p_port->p_release(p_port->p_context, ENC_CAPTURE_MASK);
    }
}
#endif /* Real firmware only. */

/*** end of file ***/


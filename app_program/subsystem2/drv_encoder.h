/** @file drv_encoder.h
 * @brief Buddy 2 GPIO inputs behind a team-owned GPIO-bank dispatcher hook.
 */
#ifndef DRV_ENCODER_H
#define DRV_ENCODER_H
#include "enc_capture.h"
typedef void (*drv_encoder_sink_t)(void * p_context, uint32_t events,
                                   uint32_t levels);
typedef struct
{
    void * p_context;
    /* reserve must NOT enable IRQ yet; it must fail if already subscribed.
     * The shared owner registers IRQ13, samples GPIO_IN once, routes only our
     * mask, acknowledges only serviced edges and never replaces other users.
     * Enable BOTH rising/falling edges on ALL four pins; baseline and enable
     * must occur with wheels stationary and motor power inhibited. Register
     * on the agreed core only; release must quiesce any in-flight callback.
     * Report dispatcher/event-loss failure via motion_api_estop(), not an
     * apparently fresh held count. Hardware edge coalescing cannot be ruled
     * out in software; measure maximum supported edge rate on target.
     */
    bool (*p_reserve)(void * p_context, uint32_t mask,
                      drv_encoder_sink_t p_sink, void * p_sink_context);
    bool (*p_enable)(void * p_context, uint32_t mask);
    void (*p_release)(void * p_context, uint32_t mask);
} drv_encoder_port_t;
/** @brief EXPLICIT owner-task setup only; missing dispatcher hooks fail closed.
 * Never registers a bank handler or touches another module's interrupt bits.
 * Caller verifies wiring/GP28 VBAT jumper and owns pins before calling.
 * Hooks must be bounded and preserve other users; called outside capture lock.
 */
bool drv_encoder_open (drv_encoder_port_t const * p_port,
    drv_encoder_sink_t p_sink, void * p_context);
void drv_encoder_close (drv_encoder_port_t const * p_port);
#endif /* DRV_ENCODER_H */

/*** end of file ***/


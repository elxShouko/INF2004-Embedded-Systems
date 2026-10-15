/**
 * @file mqtt_utk.h
 * @brief MQTT client for micro T-Kernel (RP2040, lwIP NO_SYS=1).
 *
 * The client runs entirely inside cyw43_utk_task(), the one task that owns
 * the radio and lwIP. It never blocks. The application (subsystem 1)
 * supplies the data through the hook table below, so this library has no
 * dependency on application code.
 *
 * ASSUMPTION: Every hook is called from the Wi-Fi service task only. A hook
 * must not block and must not call lwIP, MQTT or cyw43 functions.
 */

#ifndef MQTT_UTK_H
#define MQTT_UTK_H

#include <stdbool.h>
#include <stdint.h>

/** Events reported to the application through mqtt_utk_app_t.on_event. */
typedef enum
{
    MQTT_UTK_EVENT_CONNECTED = 0,      /**< Broker accepted, subscribed. */
    MQTT_UTK_EVENT_DISCONNECTED,       /**< An open session was lost. */
    MQTT_UTK_EVENT_BACKOFF,            /**< value = delay in ms. */
    MQTT_UTK_EVENT_PUBLISH_OK,         /**< A telemetry frame was sent. */
    MQTT_UTK_EVENT_PUBLISH_FAILED,     /**< lwIP refused a telemetry frame. */
    MQTT_UTK_EVENT_TELEMETRY_DISCARDED /**< Dropped while not connected. */
} mqtt_utk_event_t;

/** Hook table implemented by the application. */
typedef struct
{
    /** Build the next telemetry JSON. Return its length, or 0 if none. */
    uint32_t (*pull_telemetry)(char * p_buffer, uint32_t capacity);

    /** Build the heartbeat JSON. Return its length. */
    uint32_t (*make_heartbeat)(char * p_buffer, uint32_t capacity,
                               uint32_t uptime_ms);

    /** A complete message arrived on the command topic. */
    void (*on_command)(const char * p_payload, uint32_t length, uint32_t rx_ms);

    /** Something happened to the connection or to a frame. */
    void (*on_event)(mqtt_utk_event_t event, uint32_t value);
} mqtt_utk_app_t;

/**
 * @brief Register the application hooks. Call before the link comes up.
 * @param p_app Table that must stay valid for the life of the program.
 */
void mqtt_utk_set_app (const mqtt_utk_app_t * p_app);

/**
 * @brief Run one step of the MQTT state machine.
 * @param b_link_up True when the Wi-Fi link is up and has an address.
 * NOTE: Called by cyw43_utk_task() on every poll iteration.
 */
void mqtt_utk_poll (bool b_link_up);

#endif /* MQTT_UTK_H */

/*** end of file ***/

/* PC test double for the few lwIP MQTT calls that mqtt_utk.c uses. */
#ifndef MOCK_LWIP_MQTT_H
#define MOCK_LWIP_MQTT_H
#include <stdint.h>
typedef uint8_t u8_t; typedef uint16_t u16_t; typedef uint32_t u32_t; typedef int8_t err_t;
#define ERR_OK 0
#define ERR_MEM (-1)
#define ERR_CONN (-11)
typedef enum { MQTT_CONNECT_ACCEPTED = 0, MQTT_CONNECT_DISCONNECTED = 256, MQTT_CONNECT_TIMEOUT = 257 } mqtt_connection_status_t;
enum { MQTT_DATA_FLAG_LAST = 1 };
typedef struct { int connected; } mqtt_client_t;
struct mqtt_connect_client_info_t { const char *client_id, *client_user, *client_pass; u16_t keep_alive; const char *will_topic, *will_msg; u8_t will_qos, will_retain; };
typedef void (*mqtt_connection_cb_t)(mqtt_client_t *c, void *arg, mqtt_connection_status_t st);
typedef void (*mqtt_request_cb_t)(void *arg, err_t e);
typedef void (*mqtt_incoming_data_cb_t)(void *arg, const u8_t *d, u16_t len, u8_t flags);
typedef void (*mqtt_incoming_publish_cb_t)(void *arg, const char *topic, u32_t tot_len);
#include "lwip/ip_addr.h"
mqtt_client_t *mqtt_client_new(void);
err_t mqtt_client_connect(mqtt_client_t *c, const ip_addr_t *ip, u16_t port, mqtt_connection_cb_t cb, void *arg, const struct mqtt_connect_client_info_t *ci);
void mqtt_disconnect(mqtt_client_t *c);
u8_t mqtt_client_is_connected(mqtt_client_t *c);
err_t mqtt_publish(mqtt_client_t *c, const char *topic, const void *p, u16_t len, u8_t qos, u8_t retain, mqtt_request_cb_t cb, void *arg);
err_t mqtt_sub_unsub(mqtt_client_t *c, const char *topic, u8_t qos, mqtt_request_cb_t cb, void *arg, u8_t sub);
void mqtt_set_inpub_callback(mqtt_client_t *c, mqtt_incoming_publish_cb_t pcb, mqtt_incoming_data_cb_t dcb, void *arg);
#endif

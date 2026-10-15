/* PC stand-in for the CYW43 driver API that cyw43_utk.c uses. The test file
 * decides what each call returns. Only the names the task needs. */
#ifndef MOCK_CYW43_H
#define MOCK_CYW43_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CYW43_ITF_STA 0
#define CYW43_EIO 5
#define CYW43_AUTH_WPA2_MIXED_PSK 0x00400006u
#define CYW43_CHANNEL_NONE 0xffffffffu

#define CYW43_LINK_DOWN 0
#define CYW43_LINK_JOIN 1
#define CYW43_LINK_NOIP 2
#define CYW43_LINK_UP 3
#define CYW43_LINK_FAIL (-1)
#define CYW43_LINK_NONET (-2)
#define CYW43_LINK_BADAUTH (-3)

typedef struct
{
    uint32_t ip;
} mock_netif_t;

typedef struct
{
    uint32_t     itf_state;
    mock_netif_t netif[2];
} cyw43_t;

typedef struct
{
    int8_t  rssi;
    uint8_t ssid_len;
    uint8_t ssid[32];
    uint8_t channel;
    uint8_t auth_mode;
} cyw43_ev_scan_result_t;

extern cyw43_t cyw43_state;
extern void (*cyw43_poll)(void);

void    cyw43_init (cyw43_t * p_self);
void    cyw43_thread_enter (void);
void    cyw43_thread_exit (void);
void    cyw43_wifi_set_up (cyw43_t * p_self, int itf, bool b_up, uint32_t country);
int     cyw43_wifi_get_mac (cyw43_t * p_self, int itf, uint8_t mac[6]);
int     cyw43_wifi_join (cyw43_t * p_self, size_t ssid_len,
                         const uint8_t * p_ssid, size_t key_len,
                         const uint8_t * p_key, uint32_t auth,
                         const uint8_t * p_bssid, uint32_t channel);
int     cyw43_tcpip_link_status (cyw43_t * p_self, int itf);

#endif

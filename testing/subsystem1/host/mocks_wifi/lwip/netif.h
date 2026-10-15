#ifndef MOCK_LWIP_NETIF_H
#define MOCK_LWIP_NETIF_H
#include "../cyw43.h"
#define netif_ip4_addr(p_netif) (&(p_netif)->ip)
#define ip4_addr_get_u32(p_ip) (*(p_ip))
#endif

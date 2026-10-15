#ifndef MOCK_LWIP_IP_ADDR_H
#define MOCK_LWIP_IP_ADDR_H
#include <stdint.h>
typedef struct { uint32_t addr; } ip4_addr_t; typedef struct { ip4_addr_t ip; int type; } ip_addr_t;
#define IPADDR_TYPE_V4 0
#define ip_2_ip4(p) (&((p)->ip))
#define IP_SET_TYPE(p, t) ((p)->type = (t))
int ip4addr_aton(const char *s, ip4_addr_t *a);
#endif

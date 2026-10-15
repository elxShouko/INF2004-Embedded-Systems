#ifndef S4_FAKE_HARDWARE_TIMER_H
#define S4_FAKE_HARDWARE_TIMER_H
#include <stdint.h>
typedef struct
{
    uint32_t timerawh;
    uint32_t timerawl;
} s4_fake_timer_t;
extern s4_fake_timer_t * timer_hw;
#endif

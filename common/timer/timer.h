#pragma once

#include <stdint.h>
#include <stdbool.h>

struct timer_t
{
    uint32_t start_time;
    uint32_t timeout;
    bool elapsed;
};

int timer_init(struct timer_t *timer, uint32_t timeout);

bool timer_has_elapsed(struct timer_t *timer);
int timer_reset(struct timer_t *timer);

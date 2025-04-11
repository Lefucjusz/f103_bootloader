#include "timer.h"
#include <system.h>
#include <errno.h>
#include <stddef.h>

int timer_init(struct timer_t *timer, uint32_t timeout)
{
    if (timer == NULL) {
        return -EINVAL;
    }

    timer->start_time = system_get_ticks();
    timer->timeout = timeout;
    timer->elapsed = false;

    return 0;
}

bool timer_has_elapsed(struct timer_t *timer)
{
    if (timer == NULL) {
        return false;
    }

    /* Prevent returning true forever, return false after first check */
    if (timer->elapsed) {
        return false;
    }

    const uint32_t now = system_get_ticks();
    if ((now - timer->start_time) >= timer->timeout) {
        timer->elapsed = true;
    }

    return timer->elapsed;
}

int timer_reset(struct timer_t *timer)
{
    if (timer == NULL) {
        return -EINVAL;
    }

    return timer_init(timer, timer->timeout);
}

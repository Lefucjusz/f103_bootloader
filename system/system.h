#pragma once

#include <stdint.h>

#define SYSTEM_HSE_CONFIG RCC_CLOCK_HSE8_24MHZ
#define SYSTEM_SYSTICK_FREQ_HZ 1000 // Gives standard resolution of 1ms per tick

void system_init(void);

uint32_t system_get_ticks(void);
void system_delay_ms(uint32_t ms);

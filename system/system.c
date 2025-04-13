#include "system.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>

struct system_ctx_t
{
    volatile uint32_t ticks;
};

static struct system_ctx_t ctx;

void sys_tick_handler(void)
{
    ++ctx.ticks;
}

void system_init(void)
{
    /* Configure RCC */
    rcc_clock_setup_pll(&rcc_hsi_configs[SYSTEM_HSI_CONFIG]);

    /* Configure SysTick */
    systick_set_frequency(SYSTEM_SYSTICK_FREQ_HZ, rcc_hsi_configs[SYSTEM_HSI_CONFIG].ahb_frequency);
    systick_counter_enable();
    systick_interrupt_enable();
}

void system_deinit(void)
{
    /* Disable SysTick */
    systick_interrupt_disable();
    systick_counter_disable();
    systick_clear();
}

uint32_t system_get_ticks(void)
{
    return ctx.ticks;
}

void system_delay_ms(uint32_t ms)
{
    const uint32_t end_tick = system_get_ticks() + ms;

    while (system_get_ticks() < end_tick) {
        __asm__ __volatile__("nop\n");
    }
}

__attribute__((noreturn)) void system_panic(void)
{
    while (1) {
        __asm__ __volatile__("nop\n");
    }
}

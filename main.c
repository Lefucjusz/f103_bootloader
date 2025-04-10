#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <system.h>
#include <uart.h>

int main(void)
{
    system_init();
    uart_init();
    
    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    uint32_t last_blink_tick = 0;

    while (1) {
        if (system_get_ticks() - last_blink_tick > 500) {
            gpio_toggle(GPIOC, GPIO13);
            last_blink_tick = system_get_ticks();
        }

        if (uart_data_available()) {
            uart_write_byte(uart_read_byte());
        }
    }
    
    return 0;
}

void hard_fault_handler(void)
{
    while (1) {
        __asm__ __volatile__("nop\n");
    }
}

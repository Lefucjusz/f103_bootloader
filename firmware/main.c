#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <system.h>

int main(void)
{
    system_init();

    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);


    while (1) {
        for (int i = 50; i < 500; i += 20) {
            gpio_toggle(GPIOC, GPIO13);
            system_delay_ms(i);
        }
        for (int i = 500; i >= 50; i -= 20) {
            gpio_toggle(GPIOC, GPIO13);
            system_delay_ms(i);
        }
    }
}

void hard_fault_handler(void)
{
    system_panic();
}

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void rcc_setup(void)
{
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_24MHZ]);
}

int main(void)
{
    rcc_setup();

    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    while (1) {
        gpio_toggle(GPIOC, GPIO13);

        for (volatile int i = 0; i < 100000; ++i) {
            __asm__("nop\n");
        }
    }
    
    return 0;
}

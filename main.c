#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <system.h>
#include <uart.h>
#include <comm.h>
#include <flash.h>
#include <string.h>

int main(void)
{
    system_init();
    // uart_init();
    // comm_init();
    
    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    flash_erase_main_app();

    uint8_t data[16];
    for (size_t i = 0; i < 16; ++i) {
        data[i] = i % 2 ? 0xBA : 0xDF;
    }

    flash_write(FLASH_MAIN_APP_START, data, 5);

    memset(data, 0, sizeof(data));

    flash_read(FLASH_MAIN_APP_START, data, 5);

    while (1) {
        
    }
    
    return 0;
}

void hard_fault_handler(void)
{
    while (1) {
        __asm__ __volatile__("nop\n");
    }
}

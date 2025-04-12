#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <system.h>
#include <uart.h>
#include <comm.h>
#include <flash.h>
#include <update.h>

int main(void)
{
    system_init();
    uart_init();
    comm_init();
    
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    update_run();

    while (1) {
        
    }
    
    return 0;
}

void hard_fault_handler(void)
{
    while (1) {
        __asm__ __volatile__("bkpt #0\n");
    }
}

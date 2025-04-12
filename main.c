#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <system.h>
#include <uart.h>
#include <comm.h>
#include <update.h>
#include <boot.h>

int main(void)
{
    system_init();
    uart_init();
    comm_init();

    /* Run update procedure */
    update_run();

    /* Do not exit immediately as it causes some issues with transferring update
     * complete packet. I guess the peripherals get deinitialized too quickly. */
    system_delay_ms(100);

    /* Deinit peripherals */
    uart_deinit();
    system_deinit();

    /* Boot main app */
    boot_set_vector_table();
    boot_jump_to_firmware();
    
    return 0; // Unreachable
}

// int main(void)
// {
//     system_init();

//     rcc_periph_clock_enable(RCC_GPIOC);
//     gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);


//     while (1) {
//         gpio_toggle(GPIOC, GPIO13);
//         system_delay_ms(200);
//     }
// }

void hard_fault_handler(void)
{
    while (1) {
        __asm__ __volatile__("bkpt #0\n");
    }
}

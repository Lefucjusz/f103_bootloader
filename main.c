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

void hard_fault_handler(void)
{
    while (1) {
        __asm__ __volatile__("bkpt #0\n");
    }
}

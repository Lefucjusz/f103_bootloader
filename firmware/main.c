#include <system.h>
#include <uart.h>
#include <timer.h>
#include <string.h>
#include <stdio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void led_init(void)
{
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

static void led_toggle(void)
{
    gpio_toggle(GPIOC, GPIO13);
}

int main(void)
{
    system_init();
    led_init();
    uart_init();

    struct timer_t led_timer;
    struct timer_t uart_timer;

    timer_init(&led_timer, 250);
    timer_init(&uart_timer, 1000);

    char msg_buffer[64];
    size_t msg_counter = 0;

    while (1) {
        if (timer_has_elapsed(&led_timer)) {
            led_toggle();
            timer_reset(&led_timer);
        }

        if (timer_has_elapsed(&uart_timer)) {
            snprintf(msg_buffer, sizeof(msg_buffer), "Hello World %u from signed binary!\n", msg_counter);
            ++msg_counter;
            uart_write(msg_buffer, strlen(msg_buffer));
            timer_reset(&uart_timer);
        }
    }
}

void hard_fault_handler(void)
{
    system_panic();
}

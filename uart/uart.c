#include "uart.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>

#define UART_DATA_BITS 8

struct uart_ctx_t
{
    volatile uint8_t data_buffer;
    volatile bool data_available;
};

static struct uart_ctx_t ctx;

void usart1_isr(void)
{
    const bool is_overrun = usart_get_flag(UART_PERIPH, USART_FLAG_ORE);
    const bool data_received = usart_get_flag(UART_PERIPH, USART_FLAG_RXNE);

    if (data_received || is_overrun) {
        ctx.data_buffer = usart_recv(UART_PERIPH);
        ctx.data_available = true;
    }
}

void uart_init(void)
{
    /* Configure GPIOs */
    rcc_periph_clock_enable(UART_PORT_RCC);
    gpio_set_mode(UART_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, UART_TX_PIN);
    gpio_set_mode(UART_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, UART_RX_PIN);

    /* Enable clock for USART1 */
    rcc_periph_clock_enable(UART_PERIPH_RCC);

    /* Configure transmission parameters */
    usart_set_flow_control(UART_PERIPH, USART_FLOWCONTROL_NONE);
    usart_set_databits(UART_PERIPH, UART_DATA_BITS);
    usart_set_stopbits(UART_PERIPH, USART_STOPBITS_1);
    usart_set_parity(UART_PERIPH, USART_PARITY_NONE);
    usart_set_baudrate(UART_PERIPH, UART_BAUD_RATE);

    /* Enable full duplex mode */
    usart_set_mode(UART_PERIPH, USART_MODE_TX_RX);

    /* Enable Rx interrupt */
    usart_enable_rx_interrupt(UART_PERIPH);
    nvic_enable_irq(UART_PERIPH_IRQ);

    /* Enable USART */
    usart_enable(UART_PERIPH);
}

void uart_write(const void *data, size_t size)
{
    if (data == NULL) {
        return;
    }

    const uint8_t *data_ptr = data;

    for (size_t i = 0; i < size; ++i) {
        uart_write_byte(data_ptr[i]);
    }
}

void uart_write_byte(uint8_t data)
{
    usart_send_blocking(USART1, data);
}

size_t uart_read(void *data, size_t size)
{
    if (data == NULL) {
        return 0;
    }

    uint8_t *data_ptr = data;

    if (size > 0 && ctx.data_available) {
        data_ptr[0] = ctx.data_buffer;
        ctx.data_available = false;
    }
}

uint8_t uart_read_byte(void)
{
    ctx.data_available = false;
    return ctx.data_buffer;
}

bool uart_data_available(void)
{
    return ctx.data_available;
}

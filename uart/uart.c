#include "uart.h"
#include <ring_buffer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>

#define UART_PERIPH USART1
#define UART_PERIPH_RCC RCC_USART1
#define UART_PERIPH_IRQ NVIC_USART1_IRQ

#define UART_PORT GPIOA
#define UART_PORT_RCC RCC_GPIOA
#define UART_TX_PIN GPIO_USART1_TX
#define UART_RX_PIN GPIO_USART1_RX

#define UART_BAUD_RATE 115200
#define UART_DATA_BITS 8

#define UART_RX_BUFFER_SIZE 64

struct uart_ctx_t
{
    struct ring_buffer_t rx_buf;
    uint8_t rx_buf_data[UART_RX_BUFFER_SIZE];
};

static struct uart_ctx_t ctx;

void usart1_isr(void)
{
    const bool is_overrun = usart_get_flag(UART_PERIPH, USART_FLAG_ORE);
    const bool data_received = usart_get_flag(UART_PERIPH, USART_FLAG_RXNE);

    if (data_received || is_overrun) {
        ring_buffer_write_byte(&ctx.rx_buf, usart_recv(UART_PERIPH)); // Just ignore any errors as there's no way to recover
    }
}

void uart_init(void)
{
    /* Initialize Rx ring buffer */
    ring_buffer_init(&ctx.rx_buf, ctx.rx_buf_data, sizeof(ctx.rx_buf_data));

    /* Configure UART pins */
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

void uart_deinit(void)
{
    /* Disable USART */
    usart_disable(UART_PERIPH);

    /* Disable Rx interupt */
    usart_disable_rx_interrupt(UART_PERIPH);
    nvic_disable_irq(UART_PERIPH_IRQ);

    /* Disable UART clock */
    rcc_periph_clock_disable(UART_PERIPH_RCC);

    /* Set UART pins to inputs and disable GPIO clock */
    gpio_set_mode(UART_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, UART_TX_PIN);
    gpio_set_mode(UART_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, UART_RX_PIN);
    rcc_periph_clock_disable(UART_PORT_RCC);
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
    return ring_buffer_read(&ctx.rx_buf, data, size);
}

uint8_t uart_read_byte(void)
{
    uint8_t data;

    ring_buffer_read(&ctx.rx_buf, &data, sizeof(data));

    return data;
}

bool uart_data_available(void)
{
    return !ring_buffer_is_empty(&ctx.rx_buf);
}

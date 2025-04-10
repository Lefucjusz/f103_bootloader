#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
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

void uart_init(void);

void uart_write(const void *data, size_t size);
void uart_write_byte(uint8_t data);

size_t uart_read(void *data, size_t size);
uint8_t uart_read_byte(void);

bool uart_data_available(void);

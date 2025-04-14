#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void uart_init(void);
void uart_deinit(void);

void uart_write(const void *data, size_t size);
void uart_write_byte(uint8_t data);

size_t uart_read(void *data, size_t size);
uint8_t uart_read_byte(void);

bool uart_data_available(void);

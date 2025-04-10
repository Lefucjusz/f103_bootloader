#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct ring_buffer_t
{
    uint8_t *buffer;
    size_t write_index;
    size_t read_index;
    size_t size;
};

int ring_buffer_init(struct ring_buffer_t *rb, uint8_t *buffer, size_t size);

size_t ring_buffer_write(struct ring_buffer_t *rb, const void *data, size_t size);
int ring_buffer_write_byte(struct ring_buffer_t *rb, uint8_t data);

size_t ring_buffer_read(struct ring_buffer_t *rb, void *data, size_t size);
int ring_buffer_read_byte(struct ring_buffer_t *rb, uint8_t *data);

bool ring_buffer_is_empty(const struct ring_buffer_t *rb);

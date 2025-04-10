#include "ring_buffer.h"
#include <utils.h>
#include <errno.h>

int ring_buffer_init(struct ring_buffer_t *rb, uint8_t *buffer, size_t size)
{
    if ((rb == NULL) || (buffer == NULL)) {
        return -EINVAL;
    }

    if (!IS_POWER_OF_TWO(size)) {
        return -EINVAL;
    }

    rb->buffer = buffer;
    rb->write_index = 0;
    rb->read_index = 0;
    rb->mask = size - 1;

    return 0;
}

size_t ring_buffer_write(struct ring_buffer_t *rb, const void *data, size_t size)
{
    if ((data == NULL) || (size == 0)) {
        return 0;
    }

    const uint8_t *data_ptr = data;

    for (size_t bytes_written = 0; bytes_written < size; ++bytes_written) {
        if (ring_buffer_write_byte(rb, data_ptr[bytes_written]) != 0) {
            return bytes_written;
        }
    }

    return size;
}

int ring_buffer_write_byte(struct ring_buffer_t *rb, uint8_t data)
{
    const size_t read_index = rb->read_index;
    size_t write_index = rb->write_index;

    const size_t next_write_index = (write_index + 1) & rb->mask;
    if (next_write_index == read_index) {
        return -ENOSPC;
    }

    rb->buffer[write_index] = data;
    rb->write_index = next_write_index;

    return 0;
}

size_t ring_buffer_read(struct ring_buffer_t *rb, void *data, size_t size)
{
    if ((data == NULL) || (size == 0)) {
        return 0;
    }

    uint8_t *data_ptr = data;

    for (size_t bytes_read = 0; bytes_read < size; ++bytes_read) {
        if (ring_buffer_read_byte(rb, &data_ptr[bytes_read]) != 0) {
            return bytes_read;
        }
    }

    return size;
}

int ring_buffer_read_byte(struct ring_buffer_t *rb, uint8_t *data)
{
    if ((rb == NULL) || (data == NULL)) {
        return -EINVAL;
    }

    const size_t write_index = rb->write_index;
    size_t read_index = rb->read_index;
    if (read_index == write_index) {
        return -ENODATA;
    }

    *data = rb->buffer[read_index];
    read_index = (read_index + 1) & rb->mask;
    rb->read_index = read_index;

    return 0;
}

bool ring_buffer_is_empty(const struct ring_buffer_t *rb)
{
    if (rb == NULL) {
        return false;
    }

    return (rb->write_index == rb->read_index);
}

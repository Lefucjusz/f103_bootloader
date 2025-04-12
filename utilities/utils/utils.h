#pragma once

#include <stdint.h>

/* Generic implementation of non-reflect CRC16 */
inline static uint16_t utils_crc16(uint16_t poly, uint16_t seed, const void *data, size_t size)
{
    const uint8_t *data_ptr = data;
	uint16_t crc = seed;

	for (size_t i = 0; i < size; ++i) {
		crc ^= ((uint16_t)data_ptr[i] << 8);

		for (size_t j = 0; j < 8; ++j) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ poly;
			}
            else {
				crc = crc << 1;
			}
		}
	}

	return crc;
}

/* Implementation of CCITT-XMODEM CRC16 */
inline static uint16_t utils_crc16_xmodem(const void *data, size_t size)
{
    return utils_crc16(0x1021, 0x0000, data, size);
}

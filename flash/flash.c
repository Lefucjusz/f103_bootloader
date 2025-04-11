#include "flash.h"
#include <errno.h>
#include <libopencm3/stm32/flash.h>

void flash_erase_main_app(void)
{
    flash_unlock();

    for (size_t i = FLASH_MAIN_APP_START; i < FLASH_END_ADDR; i += FLASH_PAGE_SIZE) {
        flash_erase_page(i);
    }

    flash_lock();
}

int flash_write(size_t addr, const void *data, size_t size)
{
    /* Address has to be aligned to half word */
    if ((data == NULL) || ((addr % 2) != 0)) {
        return -EINVAL;
    }

    const uint16_t *half_word_ptr = data;
    const uint8_t *byte_ptr = data;
    const size_t half_words = size / 2;
    const bool not_aligned = size % 2;

    flash_unlock();

    /* Write complete half words */
    for (size_t i = 0; i < half_words; ++i) {
        flash_program_half_word(addr + i * 2, half_word_ptr[i]);
    }

    /* Write remaining byte if any, leave upper byte not programmed */
    if (not_aligned) {
        flash_program_half_word(addr + size - 1, byte_ptr[size - 1] | 0xFF00);
    }

    flash_lock();

    return 0;
}

void flash_read(size_t addr, void *data, size_t size)
{
    uint16_t *half_word_ptr = data;
    uint8_t *byte_ptr = data;
    const volatile uint16_t *flash_ptr = (volatile uint16_t *)addr;
    const size_t half_words = size / 2;
    const bool not_aligned = size % 2;

    /* Read complete half words */
    for (size_t i = 0; i < half_words; ++i) {
        half_word_ptr[i] = flash_ptr[i];
    }

    /* Read remaining byte if any */
    if (not_aligned) {
        byte_ptr[size - 1] = flash_ptr[half_words] & 0xFF;
    }
}

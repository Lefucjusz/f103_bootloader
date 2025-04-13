#pragma once

#include <stddef.h>

extern volatile char _bootloader_size[];

#define FLASH_PAGE_SIZE 0x400
#define FLASH_SIZE 0x10000
#define FLASH_BASE_ADDR 0x08000000
#define FLASH_END_ADDR (FLASH_BASE_ADDR + FLASH_SIZE)

#define FLASH_BOOTLOADER_START FLASH_BASE_ADDR
#define FLASH_BOOTLOADER_SIZE (size_t)_bootloader_size

#define FLASH_MAIN_APP_START (FLASH_BOOTLOADER_START + FLASH_BOOTLOADER_SIZE)
#define FLASH_MAIN_APP_MAX_SIZE (FLASH_SIZE - FLASH_BOOTLOADER_SIZE)

void flash_erase_main_app(void);

int flash_write(size_t addr, const void *data, size_t size);
void flash_read(size_t addr, void *data, size_t size);

#include "boot.h"
#include "firmware_info.h"
#include <flash.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/memorymap.h>

typedef void (*boot_entry_point_t)(void);

void boot_set_vector_table(void)
{
    SCB_VTOR = FW_VECTOR_TABLE_ENTRY_OFFSET;
}

__attribute__((noreturn)) void boot_jump_to_firmware(void)
{  
    const boot_entry_point_t fw_entry_point = *(boot_entry_point_t *)(FLASH_BASE_ADDR + FW_RESET_VECTOR_ENTRY_OFFSET);

    fw_entry_point();

    while (1); // Unreachable
}

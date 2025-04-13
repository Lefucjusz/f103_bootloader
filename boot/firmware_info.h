#pragma once

#include <flash.h>
#include <stdint.h>

#define FW_DEVICE_ID 0x69

#define FW_AES128_IV_SIZE 16
#define FW_ECDSA_SIGNATURE_SIZE 64

/* Bits [6:0] in SCB->VTOR in Cortex-M3 are reserved,
 * so the header has to be padded to multiple of 128. */
#define FW_HEADER_PADDING_SIZE 36

struct fw_header_t
{
    uint8_t aes_iv[FW_AES128_IV_SIZE];
    uint32_t version;
    uint32_t device_id;
    uint32_t length;
    uint8_t ecdsa_signature[FW_ECDSA_SIGNATURE_SIZE];
    uint8_t padding[FW_HEADER_PADDING_SIZE];
} __attribute__((packed));

#define FW_CODE_MAX_SIZE (FLASH_MAIN_APP_MAX_SIZE - sizeof(struct fw_header_t))

#define FW_VECTOR_TABLE_ENTRY_OFFSET (FLASH_BOOTLOADER_SIZE + sizeof(struct fw_header_t) + 0x00000000)
#define FW_RESET_VECTOR_ENTRY_OFFSET (FLASH_BOOTLOADER_SIZE + sizeof(struct fw_header_t) + 0x00000004)

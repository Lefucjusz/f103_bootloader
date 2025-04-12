#pragma once

#include <stdint.h>

#define FW_DEVICE_ID 0x69

#define FW_AES128_IV_SIZE 16
#define FW_RSA2048_SIGNATURE_SIZE 256

struct fw_header_t
{
    uint8_t aes_iv[FW_AES128_IV_SIZE];
    uint32_t version;
    uint32_t device_id;
    uint32_t length;
    uint8_t rsa_signature[FW_RSA2048_SIGNATURE_SIZE];
} __attribute__((packed));

#define FW_STACK_PTR_ENTRY_OFFSET (sizeof(struct fw_header_t) + 0x00000000)
#define FW_RESET_VECTOR_ENTRY_OFFSET (sizeof(struct fw_header_t) + 0x00000004)

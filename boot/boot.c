#include "boot.h"
#include "keys.h"
#include "firmware_info.h"
#include <flash.h>
#include <utils.h>
#include <sha-256.h>
#include <uECC.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/memorymap.h>

typedef void (*boot_entry_point_t)(void);

#define BOOT_FW_CHUNK_SIZE 32

static void boot_compute_fw_hash(uint8_t *hash, size_t fw_size)
{
    size_t bytes_read = 0;
    struct Sha_256 sha256;
    uint8_t buffer[BOOT_FW_CHUNK_SIZE];

    sha_256_init(&sha256, hash);

    while (bytes_read < fw_size) {
        const size_t bytes_to_read = MIN(fw_size - bytes_read, BOOT_FW_CHUNK_SIZE);

        flash_read(FW_VECTOR_TABLE_ENTRY_OFFSET + bytes_read, buffer, bytes_to_read);
        sha_256_write(&sha256, buffer, bytes_to_read);

        bytes_read += bytes_to_read;
    }

    sha_256_close(&sha256);
}

static bool boot_verify_signature(const uint8_t *fw_hash, const uint8_t *signature)
{
    const struct uECC_Curve_t *curve = uECC_secp256k1();
    const int status = uECC_verify(ecdsa_public_key, fw_hash, SIZE_OF_SHA_256_HASH, signature, curve);

    return (status != 0);
}

bool boot_verify_image(void)
{
    uint8_t fw_hash[SIZE_OF_SHA_256_HASH];
    struct fw_header_t header;

    /* Read firmware header */
    flash_read(FLASH_MAIN_APP_START, &header, sizeof(header));

    /* Check version */
    if (header.device_id != FW_DEVICE_ID) {
        return false;
    }

    /* Get firmware size and perform a basic sanity check */
    if (header.length > FW_CODE_MAX_SIZE) {
        return false;
    }

    /* Compute SHA256 of the firmware */
    boot_compute_fw_hash(fw_hash, header.length);

    /* Verify signature */
    if (!boot_verify_signature(fw_hash, header.ecdsa_signature)) {
        return false;
    }

    return true;
}

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

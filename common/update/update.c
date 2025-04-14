#include "update.h"
#include <uart.h>
#include <comm.h>
#include <timer.h>
#include <flash.h>
#include <system.h>
#include <keys.h>
#include <firmware_info.h>
#include <aes.h>
#include <string.h>

#define UPDATE_SYNC_SEQUENCE 0x33303146
#define UPDATE_SYNC_SEQUENCE_SIZE 4

#define UPDATE_TIMEOUT_MS 2000

enum update_state_t
{
    UPDATE_WAIT_FOR_SYNC,
    UPDATE_WAIT_FOR_REQUEST,
    UPDATE_GET_FW_SIZE,
    UPDATE_GET_AES_IV,
    UPDATE_GET_FW,
    UPDATE_DONE
};

union update_sync_seq_t
{
    uint32_t value;
    uint8_t raw[UPDATE_SYNC_SEQUENCE_SIZE];
};

struct update_ctx_t
{
    enum update_state_t state;
    struct timer_t timer;
    struct comm_packet_t packet;
    union update_sync_seq_t sync_seq;
    uint32_t firmware_size;
    uint32_t bytes_received;
    struct AES_ctx aes;
};

static struct update_ctx_t ctx;

static bool update_is_update_request_packet(const struct comm_packet_t *packet)
{
    if (comm_get_packet_length(packet) != COMM_REQUEST_PACKET_SIZE) {
        return false;
    }

    if (comm_get_packet_type(packet) != COMM_PACKET_CTRL) {
        return false;
    }

    if (packet->payload[0] != COMM_PACKET_OP_UPDATE_REQUEST) {
        return false;
    }

    return true;
}

static bool update_parse_fw_size_packet(const struct comm_packet_t *packet, uint32_t *fw_size)
{
    if (comm_get_packet_length(packet) != COMM_FW_SIZE_PACKET_SIZE) {
        return false;
    }

    if (comm_get_packet_type(packet) != COMM_PACKET_CTRL) {
        return false;
    }

    if (packet->payload[0] != COMM_PACKET_OP_FW_SIZE_REQUEST) {
        return false;
    }

    *fw_size = *(uint32_t *)&packet->payload[1]; // Firmware size is coded on 4 bytes
    if (*fw_size > FLASH_MAIN_APP_MAX_SIZE) {
        return false;
    }

    return true;
}

static void update_handle_failure(void)
{
    comm_create_ctrl_packet(&ctx.packet, COMM_PACKET_OP_NACK, NULL, 0); // TODO add failure reason in payload
    comm_write(&ctx.packet);

    ctx.state = UPDATE_DONE;
}

static void update_wait_for_sync(void)
{
    if (uart_data_available()) {
        memmove(&ctx.sync_seq.raw[0], &ctx.sync_seq.raw[1], UPDATE_SYNC_SEQUENCE_SIZE - 1);
        ctx.sync_seq.raw[UPDATE_SYNC_SEQUENCE_SIZE - 1] = uart_read_byte();

        if (ctx.sync_seq.value == UPDATE_SYNC_SEQUENCE) {
            const uint8_t device_id = FW_DEVICE_ID;
            comm_create_ctrl_packet(&ctx.packet, COMM_PACKET_OP_SYNCED, &device_id, sizeof(device_id));
            comm_write(&ctx.packet);

            timer_reset(&ctx.timer);
            ctx.state = UPDATE_WAIT_FOR_REQUEST;
        }
    }

    if (timer_has_elapsed(&ctx.timer)) {
        ctx.state = UPDATE_DONE;
    }
}

static void update_wait_for_request(void)
{
    if (comm_packets_available()) {
        comm_read(&ctx.packet); 
        if (!update_is_update_request_packet(&ctx.packet)) {
            update_handle_failure();
            return;
        }
        
        comm_create_ctrl_packet(&ctx.packet, COMM_PACKET_OP_ACK, NULL, 0);
        comm_write(&ctx.packet);

        timer_reset(&ctx.timer);
        ctx.state = UPDATE_GET_FW_SIZE;
    }

    if (timer_has_elapsed(&ctx.timer)) {
        update_handle_failure();
    }
}

static void update_get_fw_size(void)
{
    if (comm_packets_available()) {
        comm_read(&ctx.packet);
        if (!update_parse_fw_size_packet(&ctx.packet, &ctx.firmware_size)) {
            update_handle_failure();
            return;
        }

        comm_create_ctrl_packet(&ctx.packet, COMM_PACKET_OP_ACK, NULL, 0);
        comm_write(&ctx.packet);

        timer_reset(&ctx.timer);
        ctx.state = UPDATE_GET_AES_IV;
    }

    if (timer_has_elapsed(&ctx.timer)) {
        update_handle_failure();
    }
}

static void update_get_aes_iv(void)
{
    if (comm_packets_available()) {
        comm_read(&ctx.packet);
        if (comm_get_packet_type(&ctx.packet) != COMM_PACKET_DATA) {
            update_handle_failure();
            return;
        }

        /* Erase flash as late as possible, this way we can rollback from any previous step */
        flash_erase_main_app();

        /* First firmware packet is an IV for AES */
        AES_init_ctx_iv(&ctx.aes, aes_key, ctx.packet.payload);

        /* It's not really needed, but write it to flash anyway */
        const uint8_t packet_length = comm_get_packet_length(&ctx.packet);
        flash_write(FLASH_MAIN_APP_START + ctx.bytes_received, ctx.packet.payload, packet_length);

        comm_create_ctrl_packet(&ctx.packet, COMM_PACKET_OP_ACK, NULL, 0);
        comm_write(&ctx.packet);

        ctx.bytes_received += packet_length;
        ctx.state = UPDATE_GET_FW;
    }
    // TODO timeout
}

static void update_get_fw(void)
{
    if (comm_packets_available()) {
        comm_read(&ctx.packet);
        if (comm_get_packet_type(&ctx.packet) != COMM_PACKET_DATA) {
            update_handle_failure();
            return;
        }

        const uint8_t packet_length = comm_get_packet_length(&ctx.packet);

        AES_CBC_decrypt_buffer(&ctx.aes, ctx.packet.payload, packet_length);
        flash_write(FLASH_MAIN_APP_START + ctx.bytes_received, ctx.packet.payload, packet_length);

        ctx.bytes_received += packet_length;
        if (ctx.bytes_received < ctx.firmware_size) {
            comm_create_ctrl_packet(&ctx.packet, COMM_PACKET_OP_ACK, NULL, 0);
            comm_write(&ctx.packet);
        }
        else {
            comm_create_ctrl_packet(&ctx.packet, COMM_PACKET_OP_FW_UPDATE_DONE, NULL, 0);
            comm_write(&ctx.packet);

            ctx.state = UPDATE_DONE;
        }
    }
    // TODO timeout
}

void update_run(void)
{
    timer_init(&ctx.timer, UPDATE_TIMEOUT_MS);

    ctx.state = UPDATE_WAIT_FOR_SYNC;

    while (ctx.state != UPDATE_DONE) {
        switch (ctx.state) {
            case UPDATE_WAIT_FOR_SYNC:
                update_wait_for_sync();
                break;

            case UPDATE_WAIT_FOR_REQUEST:
                update_wait_for_request();
                break;

            case UPDATE_GET_FW_SIZE:
                update_get_fw_size();
                break;

            case UPDATE_GET_AES_IV:
                update_get_aes_iv();
                break;

            case UPDATE_GET_FW:
                update_get_fw();
                break;

            default:
                break;
        }

        /* Prevent communication handler consuming UART bytes while waiting for sync */
        if (ctx.state != UPDATE_WAIT_FOR_SYNC) {
            comm_task();
        }
    }
}

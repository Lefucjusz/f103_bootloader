#include "comm.h"
#include <uart.h>
#include <utils.h>
#include <ring_buffer.h>
#include <string.h>
#include <errno.h>

#define COMM_PACKET_BUFFER_COUNT 4
#define COMM_PACKET_BUFFER_SIZE (COMM_PACKET_BUFFER_COUNT * sizeof(struct comm_packet_t))

#define COMM_PACKET_LENGTH_SHIFT 0
#define COMM_PACKET_LENGTH_MASK (0x1F << COMM_PACKET_LENGTH_SHIFT)

#define COMM_PACKET_TYPE_SHIFT 5
#define COMM_PACKET_TYPE_MASK (0x07 << COMM_PACKET_TYPE_SHIFT)

enum comm_state_t
{
    COMM_RECEIVE_METADATA = 0,
    COMM_RECEIVE_DATA,
    COMM_RECEIVE_CRC,
    COMM_PROCESS_PACKET
};

struct comm_ctx_t
{
    enum comm_state_t state;
    uint8_t rx_count;
    struct comm_packet_t last_tx_packet;
    struct comm_packet_t current_rx_packet;
    struct comm_packet_t retx_packet;
    struct ring_buffer_t packet_buffer;
    uint8_t packet_buffer_data[COMM_PACKET_BUFFER_SIZE];
};

static struct comm_ctx_t ctx;

static bool comm_is_retx_packet(const struct comm_packet_t *packet)
{
    return (memcmp(packet, &ctx.retx_packet, COMM_PACKET_TOTAL_SIZE) == 0);
}

int comm_init(void)
{
    /* Initialize packet ring buffer */
    const int err = ring_buffer_init(&ctx.packet_buffer, ctx.packet_buffer_data, sizeof(ctx.packet_buffer_data));
    if (err) {
        return err;
    }

    /* Create retransmit packet */
    comm_create_ctrl_packet(&ctx.retx_packet, COMM_PACKET_OP_RETX, NULL, 0);

    return 0;
}

void comm_write(const struct comm_packet_t *packet)
{
    if (packet == NULL) {
        return;
    }

    uart_write(packet, COMM_PACKET_TOTAL_SIZE);
    ctx.last_tx_packet = *packet;
}

void comm_read(struct comm_packet_t *packet)
{
    ring_buffer_read(&ctx.packet_buffer, packet, COMM_PACKET_TOTAL_SIZE);
}

bool comm_packets_available(void)
{
    return !ring_buffer_is_empty(&ctx.packet_buffer);
}

uint16_t comm_compute_crc(const struct comm_packet_t *packet)
{
    return utils_crc16_xmodem(packet, COMM_PACKET_TOTAL_SIZE - COMM_PACKET_CRC16_SIZE);
}

int comm_set_packet_type(struct comm_packet_t *packet, enum comm_packet_type_t type)
{
    if (packet == NULL) {
        return -EINVAL;
    }

    if ((type < 0) || (type >= COMM_PACKET_COUNT)) {
        return -EINVAL;
    }

    packet->metadata &= ~COMM_PACKET_TYPE_MASK;
    packet->metadata |= (type << COMM_PACKET_TYPE_SHIFT) & COMM_PACKET_TYPE_MASK;

    return 0;
}

enum comm_packet_type_t comm_get_packet_type(const struct comm_packet_t *packet)
{
    if (packet == NULL) {
        return -EINVAL;
    }

    return (packet->metadata & COMM_PACKET_TYPE_MASK) >> COMM_PACKET_TYPE_SHIFT;
}

int comm_set_packet_length(struct comm_packet_t *packet, uint8_t length)
{
    if (packet == NULL) {
        return -EINVAL;
    }

    if (length > COMM_PACKET_PAYLOAD_SIZE) {
        return -EINVAL;
    }

    packet->metadata &= ~COMM_PACKET_LENGTH_MASK;
    packet->metadata |= (length << COMM_PACKET_LENGTH_SHIFT) & COMM_PACKET_LENGTH_MASK;

    return 0;
}

uint8_t comm_get_packet_length(const struct comm_packet_t *packet)
{
    if (packet == NULL) {
        return -EINVAL;
    }

    return (packet->metadata & COMM_PACKET_LENGTH_MASK) >> COMM_PACKET_LENGTH_SHIFT;
}

int comm_create_ctrl_packet(struct comm_packet_t *packet, enum comm_packet_op_t op, const void *payload, size_t payload_size)
{
    if (packet == NULL) {
        return -EINVAL;
    }

    if (payload_size >= COMM_PACKET_PAYLOAD_SIZE) {
        return -E2BIG;
    }

    comm_set_packet_length(packet, payload_size + 1);
    comm_set_packet_type(packet, COMM_PACKET_CTRL);

    const size_t padding_size = COMM_PACKET_PAYLOAD_SIZE - payload_size - 1;
    packet->payload[0] = op;
    if (payload != NULL) {  
        memcpy(&packet->payload[1], payload, payload_size);
    }
    memset(&packet->payload[payload_size + 1], COMM_PACKET_PADDING_BYTE, padding_size);

    packet->crc.value = comm_compute_crc(packet);

    return 0;
}

void comm_task(void)
{
    while (uart_data_available() || (ctx.state == COMM_PROCESS_PACKET)) {
        switch (ctx.state) {
            case COMM_RECEIVE_METADATA:
                ctx.current_rx_packet.metadata = uart_read_byte();
                ctx.state = COMM_RECEIVE_DATA;
                break;

            case COMM_RECEIVE_DATA:
                ctx.current_rx_packet.payload[ctx.rx_count] = uart_read_byte();
                ++ctx.rx_count;
                if (ctx.rx_count >= COMM_PACKET_PAYLOAD_SIZE) {
                    ctx.rx_count = 0;
                    ctx.state = COMM_RECEIVE_CRC;
                }
                break;

            case COMM_RECEIVE_CRC:
                ctx.current_rx_packet.crc.raw[ctx.rx_count] = uart_read_byte();
                ++ctx.rx_count;
                if (ctx.rx_count >= sizeof(ctx.current_rx_packet.crc)) {
                    ctx.rx_count = 0;
                    ctx.state = COMM_PROCESS_PACKET;
                }
                break;

            case COMM_PROCESS_PACKET: {
                /* Validate CRC */
                const uint16_t computed_crc = comm_compute_crc(&ctx.current_rx_packet);
                if (ctx.current_rx_packet.crc.value != computed_crc) {
                    comm_write(&ctx.retx_packet);
                    ctx.state = COMM_RECEIVE_METADATA;
                    break;
                }

                /* Handle retransmit request */
                if (comm_is_retx_packet(&ctx.current_rx_packet)) {
                    comm_write(&ctx.last_tx_packet);
                    ctx.state = COMM_RECEIVE_METADATA;
                    break;
                }

                /* Handle data packet */
                const size_t written = ring_buffer_write(&ctx.packet_buffer, &ctx.current_rx_packet, COMM_PACKET_TOTAL_SIZE);
                if (written != COMM_PACKET_TOTAL_SIZE) {
                    __asm__ __volatile__("bkpt #0\n"); // TODO just for debug
                }

                ctx.state = COMM_RECEIVE_METADATA;
            } break;

            default:
                /* We should never get here, but just in case */
                ctx.state = COMM_RECEIVE_METADATA;
                break;
        }
    }
}

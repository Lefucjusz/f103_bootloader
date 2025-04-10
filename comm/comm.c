#include "comm.h"
#include <uart.h>
#include <utils.h>
#include <ring_buffer.h>
#include <string.h>

#define COMM_PACKET_BUFFER_COUNT 8
#define COMM_PACKET_BUFFER_SIZE (COMM_PACKET_BUFFER_COUNT * sizeof(struct comm_packet_t))

enum comm_state_t
{
    COMM_RECEIVE_LENGTH = 0,
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
    struct comm_packet_t ack_packet;
    struct ring_buffer_t packet_buffer;
    uint8_t packet_buffer_data[COMM_PACKET_BUFFER_SIZE];
};

static struct comm_ctx_t ctx;

// TODO maybe rewrite memcmp and memset if it saves flash

static bool comm_is_retx_packet(const struct comm_packet_t *packet)
{
    return (memcmp(packet, &ctx.retx_packet, COMM_PACKET_TOTAL_SIZE) == 0);
}

static bool comm_is_ack_packet(const struct comm_packet_t *packet)
{
    return (memcmp(packet, &ctx.ack_packet, COMM_PACKET_TOTAL_SIZE) == 0);
}

int comm_init(void)
{
    /* Initialize packet ring buffer */
    const int err = ring_buffer_init(&ctx.packet_buffer, ctx.packet_buffer_data, sizeof(ctx.packet_buffer_data));
    if (err) {
        return err;
    }

    /* Create retransmission request packet */
    ctx.retx_packet.length = 1;
    ctx.retx_packet.payload[0] = COMM_PACKET_RETX_PAYLOAD;
    (void)memset(&ctx.retx_packet.payload[1], COMM_PACKET_PADDING_BYTE, COMM_PACKET_PAYLOAD_SIZE - 1);
    ctx.retx_packet.crc.value = comm_compute_crc(&ctx.retx_packet);

    /* Create acknowledge packet */
    ctx.ack_packet.length = 1;
    ctx.ack_packet.payload[0] = COMM_PACKET_ACK_PAYLOAD;
    (void)memset(&ctx.ack_packet.payload[1], COMM_PACKET_PADDING_BYTE, COMM_PACKET_PAYLOAD_SIZE - 1);
    ctx.ack_packet.crc.value = comm_compute_crc(&ctx.ack_packet);

    return 0;
}

void comm_write(const struct comm_packet_t *packet)
{
    uart_write(packet, COMM_PACKET_TOTAL_SIZE);
}

void comm_read(struct comm_packet_t *packet)
{
    (void)ring_buffer_read(&ctx.packet_buffer, packet, COMM_PACKET_TOTAL_SIZE);
}

bool comm_packets_available(void)
{
    return !ring_buffer_is_empty(&ctx.packet_buffer);
}

uint16_t comm_compute_crc(const struct comm_packet_t *packet)
{
    return crc16_xmodem(&ctx.current_rx_packet, COMM_PACKET_TOTAL_SIZE - COMM_PACKET_CRC16_SIZE);
}

void comm_task(void)
{
    while (uart_data_available()) {
        switch (ctx.state) {
            case COMM_RECEIVE_LENGTH:
                ctx.current_rx_packet.length = uart_read_byte();
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
                    ctx.state = COMM_RECEIVE_LENGTH;
                    break;
                }

                /* Handle retransmit request */
                if (comm_is_retx_packet(&ctx.current_rx_packet)) {
                    comm_write(&ctx.last_tx_packet);
                    ctx.state = COMM_RECEIVE_LENGTH;
                    break;
                }

                /* Handle acknowledge packet */
                if (comm_is_ack_packet(&ctx.current_rx_packet)) {
                    ctx.state = COMM_RECEIVE_LENGTH;
                    break;
                }

                /* Handle data packet */
                const size_t written = ring_buffer_write(&ctx.packet_buffer, &ctx.current_rx_packet, COMM_PACKET_TOTAL_SIZE);
                if (written == COMM_PACKET_TOTAL_SIZE) {
                    __asm__ __volatile__("bkpt #0\n"); // TODO just for debug
                }

                comm_write(&ctx.ack_packet);
                ctx.state = COMM_RECEIVE_LENGTH;
            } break;

            default:
                /* We should never get here, but just in case */
                ctx.state = COMM_RECEIVE_LENGTH;
                break;
        }
    }
}

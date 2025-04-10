#pragma once

#include <stdint.h>
#include <stdbool.h>

#define COMM_PACKET_LENGTH_SIZE 1
#define COMM_PACKET_PAYLOAD_SIZE 16
#define COMM_PACKET_CRC16_SIZE 2
#define COMM_PACKET_TOTAL_SIZE (COMM_PACKET_LENGTH_SIZE + COMM_PACKET_PAYLOAD_SIZE + COMM_PACKET_CRC16_SIZE)

#define COMM_PACKET_ACK_PAYLOAD 0x06
#define COMM_PACKET_RETX_PAYLOAD 0x18

#define COMM_PACKET_PADDING_BYTE 0xFF

// enum comm_packet_type_t
// {
//     COMM_PACKET_DATA = 0,
//     COMM_PACKET_CTRL
// }; // TODO implement this

struct comm_packet_t
{
    uint8_t length;
    uint8_t payload[COMM_PACKET_PAYLOAD_SIZE];
    union {
        uint16_t value;
        uint8_t raw[COMM_PACKET_CRC16_SIZE];
    } crc;
} __attribute__((packed));

int comm_init(void);

void comm_write(const struct comm_packet_t *packet);
void comm_read(struct comm_packet_t *packet);

bool comm_packets_available(void);
uint16_t comm_compute_crc(const struct comm_packet_t *packet);

void comm_task(void);

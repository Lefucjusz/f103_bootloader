#pragma once

#include <stdint.h>
#include <stdbool.h>

#define COMM_PACKET_METADATA_SIZE 1
#define COMM_PACKET_PAYLOAD_SIZE 16
#define COMM_PACKET_CRC16_SIZE 2
#define COMM_PACKET_TOTAL_SIZE (COMM_PACKET_METADATA_SIZE + COMM_PACKET_PAYLOAD_SIZE + COMM_PACKET_CRC16_SIZE)

#define COMM_PACKET_PADDING_BYTE 0xFF

enum comm_packet_type_t
{
    COMM_PACKET_DATA = 0,
    COMM_PACKET_CTRL,
    COMM_PACKET_COUNT
};

enum comm_packet_op_t
{
    COMM_PACKET_OP_ACK = 0x06,
    COMM_PACKET_OP_RETX = 0x18
};

struct comm_packet_t
{
    uint8_t metadata;
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

int comm_set_packet_type(struct comm_packet_t *packet, enum comm_packet_type_t type);
int comm_set_packet_length(struct comm_packet_t *packet, uint8_t length);

void comm_task(void);

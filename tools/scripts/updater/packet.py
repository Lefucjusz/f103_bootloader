from enum import Enum, IntEnum

class Packet:
    class Type(IntEnum):
        DATA = 0
        CONTROL = 1

    class Operation(Enum):
        FW_UPDATE_DONE = b'\x04'
        ACK = b'\x06'
        UPDATE_REQUEST = b'\x11'
        FW_SIZE_REQUEST = b'\x12'
        NACK = b'\x15'
        SYNCED = b'\x16'
        RETX = b'\x18'

    LENGTH_SHIFT = 0
    LENGTH_MASK = 0x1F << LENGTH_SHIFT

    TYPE_SHIFT = 5
    TYPE_MASK = 0x07 << TYPE_SHIFT

    METADATA_SIZE = 1
    PAYLOAD_SIZE = 16
    CRC_SIZE = 2
    TOTAL_SIZE = METADATA_SIZE + PAYLOAD_SIZE + CRC_SIZE

    CONTROL_PACKET_LENGTH = 1

    PADDING_BYTE = 0xFF


    def __init__(self, payload: bytes = bytes(), type: Type = Type.DATA, crc: int | None = None):
        self.payload = payload
        self.pad_payload()

        self.metadata = 0
        self.set_type(type)
        self.set_length(len(payload))

        if crc is None:
            self.crc = self.compute_crc()
        else:
            self.crc = crc


    def from_bytes(self, data: bytes) -> None:
        if len(data) != self.TOTAL_SIZE:
            raise Exception(f'Packet must consist of exactly {self.TOTAL_SIZE} bytes, got {len(data)}')

        self.metadata = int(data[0])
        self.payload = data[self.METADATA_SIZE : self.METADATA_SIZE + self.PAYLOAD_SIZE]
        self.crc = int.from_bytes(data[self.METADATA_SIZE + self.PAYLOAD_SIZE:], 'little')


    def pad_payload(self):
        pad_length = self.PAYLOAD_SIZE - len(self.payload)
        if pad_length > 0:
            self.payload += self.PADDING_BYTE.to_bytes(1, 'little') * pad_length


    def set_type(self, type: Type) -> None:
        self.metadata &= ~self.TYPE_MASK
        self.metadata |= (type << self.TYPE_SHIFT) & self.TYPE_MASK


    def set_length(self, length: int) -> None:
        self.metadata &= ~self.LENGTH_MASK
        self.metadata |= (length << self.LENGTH_SHIFT) & self.LENGTH_MASK


    def compute_crc(self) -> int:
        meta_byte = self.metadata.to_bytes(1, 'little')
        return self.crc16_xmodem(meta_byte + self.payload)


    def is_valid(self) -> bool:
        if self.get_length() > self.PAYLOAD_SIZE:
            return False
        if self.crc != self.compute_crc():
            return False
        return True


    def is_operation(self, operation: Operation) -> bool:
        if self.get_type() != self.Type.CONTROL:
            return False
        if self.get_operation() != operation:
            return False
        return True


    def get_type(self) -> Type:
        return (self.metadata & self.TYPE_MASK) >> self.TYPE_SHIFT


    def get_length(self) -> int:
        return (self.metadata & self.LENGTH_MASK) >> self.LENGTH_SHIFT


    def get_operation(self) -> Operation:
        return self.Operation(self.payload[:1])


    def get_payload(self) -> bytes:
        length = self.get_length()
        return self.payload[:length]


    def get_crc(self) -> int:
        return self.crc


    def get_raw(self) -> bytes:
        meta_byte = self.metadata.to_bytes(1, 'little')
        crc_bytes = self.crc.to_bytes(2, 'little')
        return meta_byte + self.payload + crc_bytes


    def crc16_xmodem(self, data: bytes) -> bytes:
        crc = 0

        for byte in data:
            crc ^= (byte << 8) & 0xFFFF

            for _ in range(8):
                if crc & 0x8000:
                    crc = ((crc << 1) ^ 0x1021) & 0xFFFF
                else:
                    crc = (crc << 1) & 0xFFFF

        return crc

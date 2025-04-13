import serial
import time
from packet import Packet
from enum import IntEnum
import os
import math

class Updater:
    class UpdateState(IntEnum):
        SYNC = 0
        VALIDATE_ID = 1
        ACK_UPDATE = 2
        ACK_FW_SIZE = 3
        SEND_FW_DATA = 4
        ACK_DATA = 5
        DONE = 6

    BAUDRATE = 115200
    SYNC_SEQUENCE = b'\x46\x31\x30\x33'

    def __init__(self):
        self.rx_buffer = bytes()
        self.rx_packets = []
        self.last_tx_packet = Packet()
        self.state = self.UpdateState.SYNC

    def print_packet_data(self, packet: Packet) -> None:
        print(f'Type: {packet.get_type()}')
        print(f'Payload: {packet.get_payload()}')
        print(f'Length: {packet.get_length()}')
        print(f'CRC16: {packet.get_crc()}\n')


    def print_progress(self) -> None:
        current_pos = self.file.tell()
        chunks_total = math.ceil(self.file_size / Packet.PAYLOAD_SIZE)
        current_chunk = math.ceil(current_pos / Packet.PAYLOAD_SIZE) + 1
        print(f'Sending chunk {current_chunk}/{chunks_total}', end='\r')


    def packets_available(self) -> bool:
        return len(self.rx_packets) > 0


    def validate_device_id(self, packet: Packet) -> bool:
        if not packet.is_operation(Packet.Operation.SYNCED):
            return False
        if packet.get_payload()[1:2] != self.device_id:
            return False
        return True


    def rx_callback(self, data: bytes) -> None:
        # Append new data to buffer
        self.rx_buffer += data

        while len(self.rx_buffer) >= Packet.TOTAL_SIZE:
            # Create new packet
            packet = Packet()
            packet.from_bytes(self.rx_buffer[:Packet.TOTAL_SIZE])

            # Remove current packet data from buffer
            self.rx_buffer = self.rx_buffer[Packet.TOTAL_SIZE:]

            # Validate packet
            if not packet.is_valid():
                print('Got invalid packet, requesting retransmission')
                self.port.write(Packet(Packet.Operation.RETX.value, Packet.Type.CONTROL).get_raw())
            elif packet.is_operation(Packet.Operation.RETX):
                print('Requested retransmission of last packet')
                self.port.write(self.last_tx_packet.get_raw())
            else:
                self.rx_packets.append(packet)
                # self.print_packet_data(packet)


    def update_handler(self) -> None:
        match self.state:
            case self.UpdateState.SYNC:
                if self.packets_available():
                    packet = self.rx_packets.pop(0)
                    if not self.validate_device_id(packet):
                        print('Failed to validate device ID!')
                        self.state = self.UpdateState.DONE
                    else:
                        print('Device ID valid, requesting update...')
                        self.port.write(Packet(Packet.Operation.UPDATE_REQUEST.value, Packet.Type.CONTROL).get_raw())
                        self.state = self.UpdateState.ACK_UPDATE
                else:
                    print('Sending sync sequence...')
                    self.port.write(self.SYNC_SEQUENCE)
                    time.sleep(0.5)

            case self.UpdateState.ACK_UPDATE:
                if self.packets_available():
                    packet = self.rx_packets.pop(0)
                    if not packet.is_operation(Packet.Operation.ACK):
                        print('Failed to get update confirmation!')
                        self.state = self.UpdateState.DONE
                    else:
                        print('Update request confirmed, sending firmware size...')
                        packet_data = Packet.Operation.FW_SIZE_REQUEST.value + int.to_bytes(self.file_size, 4, 'little')
                        self.port.write(Packet(packet_data, Packet.Type.CONTROL).get_raw())
                        self.state = self.UpdateState.ACK_FW_SIZE

            case self.UpdateState.ACK_FW_SIZE:
                if self.packets_available():
                    packet = self.rx_packets.pop(0)
                    if not packet.is_operation(Packet.Operation.ACK):
                        print('Failed to get firmware size confirmation!')
                        self.state = self.UpdateState.DONE
                    else:
                        print('Firmware size confirmed, sending firmware...')
                        self.state = self.UpdateState.SEND_FW_DATA

            case self.UpdateState.SEND_FW_DATA:
                self.print_progress()
                chunk = self.file.read(Packet.PAYLOAD_SIZE)
                self.port.write(Packet(chunk).get_raw())
                self.state = self.UpdateState.ACK_DATA

            case self.UpdateState.ACK_DATA:
                if self.packets_available():
                    packet = self.rx_packets.pop(0)
                    if packet.is_operation(Packet.Operation.ACK):
                        self.state = self.UpdateState.SEND_FW_DATA
                    elif packet.is_operation(Packet.Operation.FW_UPDATE_DONE):
                        print("\nUpdate done!")
                        self.state = self.UpdateState.DONE
                    else:
                        print("\nFailed to get ACK!")
                        self.state = self.UpdateState.DONE


    def run(self, port_path: str, file_path: str, device_id: bytes) -> None:
        self.device_id = device_id
        self.file = open(file_path, "rb")
        self.file_size = os.path.getsize(file_path)
        self.port = serial.Serial(port_path, baudrate=self.BAUDRATE, timeout=1)

        while self.state != self.UpdateState.DONE:
            if self.port.in_waiting > 0:
                data = self.port.read_all()
                self.rx_callback(data)
            self.update_handler()
            time.sleep(0.01)

        self.port.close()
        self.file.close()

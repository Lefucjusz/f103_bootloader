from cryptography.hazmat.primitives import hashes, serialization, padding
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature, encode_dss_signature
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.exceptions import InvalidSignature
import secrets
import argparse

SIGNATURE_SIZE = 64

AES_BLOCK_SIZE_BITS = 128 # AES128
AES_BLOCK_SIZE = AES_BLOCK_SIZE_BITS // 8

# FW header has to be multiple of 128 due to SCB->VTOR bits [6:0] being unused
HEADER_SIZE = 128
HEADER_PADDING_SIZE = 36
HEADER_PADDING_BYTE = b'\xFF'


def sign(firmware_path: str, signed_firmware_path: str, aes_key_path: str, private_key_path: str, version: int, device_id: int) -> None:
    # Read firmware data and remove header placeholder
    with open(firmware_path, 'rb') as f:
        firmware_data = f.read()
    firmware_data = firmware_data[HEADER_SIZE:]

    # Compute SHA256 of firmware data and sign it with ECDSA
    with open(private_key_path, 'rb') as f:
        key = serialization.load_pem_private_key(f.read(), password=None)
    signature = key.sign(firmware_data, ec.ECDSA(hashes.SHA256()))

    # Convert signature from DER format to raw bytes
    r, s = decode_dss_signature(signature)
    signature = r.to_bytes(32, 'big') + s.to_bytes(32, 'big')

    # Add header to firmware data
    version_data = version.to_bytes(4, 'little')
    id_data = device_id.to_bytes(4, 'little')
    size_data = len(firmware_data).to_bytes(4, 'little')
    padding_data = HEADER_PADDING_BYTE * HEADER_PADDING_SIZE
    firmware_data = version_data + id_data + size_data + signature + padding_data + firmware_data

    # Pad for AES128
    padder = padding.PKCS7(AES_BLOCK_SIZE_BITS).padder()
    firmware_data = padder.update(firmware_data) + padder.finalize()

    # Read AES128 key
    with open(aes_key_path, 'rb') as f:
        aes_key = f.read()
    if len(aes_key) != AES_BLOCK_SIZE:
        print('Invalid AES128 key size!')
        return

    # Generate IV and encrypt the firmware
    iv = secrets.token_bytes(AES_BLOCK_SIZE)
    cipher = Cipher(algorithms.AES(aes_key), modes.CBC(iv))
    encryptor = cipher.encryptor()
    encrypted_firmware_data = encryptor.update(firmware_data) + encryptor.finalize()

    # Add IV at the beginning and save to file
    with open(signed_firmware_path, 'wb') as f:
        f.write(iv + encrypted_firmware_data)


def verify(signed_firmware_path: str, aes_key_path: str, public_key_path: str):
    # Read encrypted firmware data
    with open(signed_firmware_path, 'rb') as f:
        encrypted_firmware_data = f.read()

    # Get IV from data
    iv = encrypted_firmware_data[:AES_BLOCK_SIZE]
    encrypted_firmware_data = encrypted_firmware_data[AES_BLOCK_SIZE:]

    # Read AES128 key
    with open(aes_key_path, 'rb') as f:
        aes_key = f.read()
    if len(aes_key) != AES_BLOCK_SIZE:
        print('Invalid AES128 key size!')
        return

    # Decrypt firmware
    cipher = Cipher(algorithms.AES(aes_key), modes.CBC(iv))
    decryptor = cipher.decryptor()
    firmware_data = decryptor.update(encrypted_firmware_data)

    # Get header from file
    version = int.from_bytes(firmware_data[:4], 'little')
    firmware_data = firmware_data[4:]
    id = int.from_bytes(firmware_data[:4], 'little')
    firmware_data = firmware_data[4:]
    size = int.from_bytes(firmware_data[:4], 'little')
    firmware_data = firmware_data[4:]

    print(f'Version: {version}')
    print(f'ID: 0x{id:02X}')
    print(f'Size: {size}B')

    # Get signature from data
    signature = firmware_data[:SIGNATURE_SIZE]
    firmware_data = firmware_data[SIGNATURE_SIZE:]

    # Skip header padding
    firmware_data = firmware_data[HEADER_PADDING_SIZE:]

    # Remove AES padding
    firmware_data = firmware_data[:size]

    # Get public ECDSA key and verify signature
    with open(public_key_path, 'rb') as f:
        key = serialization.load_pem_public_key(f.read())

    r = int.from_bytes(signature[:32], 'big')
    s = int.from_bytes(signature[32:], 'big')
    der_signature = encode_dss_signature(r, s)

    try:
        key.verify(der_signature, firmware_data, ec.ECDSA(hashes.SHA256()))
        print('Signature valid!')
    except InvalidSignature:
        print('Signature invalid!')


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('firmware_path', help='path to compiled binary to be signed', type=str)
    parser.add_argument('output_path', help='path to output signed binary file', type=str)
    parser.add_argument('aes_key_path', help='path to AES128 key to use to encrypt the firmware', type=str)
    parser.add_argument('private_key_path', help='path to private ECDSA key in PEM format to use to sign the firmware', type=str)
    parser.add_argument('public_key_path', help='path to public ECDSA key in PEM format to use to verify the signature', type=str)
    parser.add_argument('version', help='firmware version number', type=int)
    parser.add_argument('device_id', help='ID of the device this firmware is for', type=str)
    args = parser.parse_args()

    if args.device_id.startswith('0x'):
        device_id = int(args.device_id, 16)
    else:
        device_id = int(args.device_id)

    print('Signing...')
    sign(args.firmware_path, args.output_path, args.aes_key_path, args.private_key_path, args.version, device_id)
    print('Firmware signed! Performing verification...')
    verify(args.output_path, args.aes_key_path, args.public_key_path)


if __name__ == '__main__':
    main()

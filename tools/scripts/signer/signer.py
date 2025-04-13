import os
import secrets
from cryptography.hazmat.primitives import hashes, serialization, padding
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature, encode_dss_signature
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.exceptions import InvalidSignature

FW_FILE_PATH = '../../../build/firmware.bin' # TODO take these from command line
SIGNED_FW_FILE_PATH = '../../../build/signed.bin'
PRIVATE_KEY_PATH = '../../keys/private.pem'
PUBLIC_KEY_PATH = '../../keys/public.pem'

SIGNATURE_SIZE = 64

AES_BLOCK_SIZE_BITS = 128 # AES128
AES_BLOCK_SIZE = AES_BLOCK_SIZE_BITS // 8
AES_KEY = '0123456789ABCDEF'

# FW header has to be multiple of 128 due to SCB->VTOR bits [6:0] being unused
HEADER_SIZE = 128
HEADER_PADDING_SIZE = 36
HEADER_PADDING_BYTE = b'\xFF'


def sign(firmware_path: str, signed_firmware_path: str, private_key_path: str, version: int, device_id: int) -> None:
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

    # Generate IV and encrypt the firmware
    iv = secrets.token_bytes(AES_BLOCK_SIZE)
    cipher = Cipher(algorithms.AES(AES_KEY.encode()), modes.CBC(iv))
    encryptor = cipher.encryptor()
    encrypted_firmware_data = encryptor.update(firmware_data) + encryptor.finalize()

    # Add IV at the beginning and save to file
    with open(signed_firmware_path, 'wb') as f:
        f.write(iv + encrypted_firmware_data)


def validate(signed_firmware_path: str, public_key_path: str):
    # Read encrypted firmware data
    with open(signed_firmware_path, 'rb') as f:
        encrypted_firmware_data = f.read()

    # Get IV from data
    iv = encrypted_firmware_data[:AES_BLOCK_SIZE]
    encrypted_firmware_data = encrypted_firmware_data[AES_BLOCK_SIZE:]

    # Decrypt firmware
    cipher = Cipher(algorithms.AES(AES_KEY.encode()), modes.CBC(iv))
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
    print(f'ID: {id}')
    print(f'Size: {size}')

    # Get signature from data
    signature = firmware_data[:SIGNATURE_SIZE]
    firmware_data = firmware_data[SIGNATURE_SIZE:]

    # Skip padding
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
        print("Signature valid!")
    except InvalidSignature:
        print("Signature invalid!")


if __name__ == "__main__":
    sign(FW_FILE_PATH, SIGNED_FW_FILE_PATH, PRIVATE_KEY_PATH, 0, 0x69)
    validate(SIGNED_FW_FILE_PATH, PUBLIC_KEY_PATH)

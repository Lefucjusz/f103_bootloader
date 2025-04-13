from cryptography.hazmat.primitives import serialization

KEY_PATH = 'public.pem'
OUTPUT_PATH = 'public_bin.txt'


def main(public_pem_path: int, bin_array_path: int) -> None:
    with open(public_pem_path, 'rb') as f:
        key = serialization.load_pem_public_key(f.read())

    raw_bytes = key.public_bytes(serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)

    if raw_bytes[0] == 'x\04':
        print('Invalid prefix!')
        return
    
    # Strip prefix
    raw_bytes = raw_bytes[1:]

    # Print to file in 8x8 array alignment
    with open(bin_array_path, 'w') as f:
        f.write('static const uint8_t ecdsa_public_key[] = {\n\t')
        for i, byte in enumerate(raw_bytes):
            if (i + 1) % 8 != 0:
                f.write(f'0x{byte:02X}, ')
            elif i != len(raw_bytes) - 1:
                f.write(f'0x{byte:02X},\n\t')
            else:
                f.write(f'0x{byte:02X}')
        f.write('\n};')


if __name__ == '__main__':
    main(KEY_PATH, OUTPUT_PATH)

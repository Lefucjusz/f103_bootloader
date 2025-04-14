from cryptography.hazmat.primitives import serialization
import argparse


def convert_key(public_pem_path: int, bin_array_path: int) -> None:
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


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('source', help='path to public key in PEM format', type=str)
    parser.add_argument('output', help='path to output file with key in C array format', type=str)
    args = parser.parse_args()

    convert_key(args.source, args.output)

    print(f'Conversion done! Raw key data has been saved to {args.output}')


if __name__ == '__main__':
    main()

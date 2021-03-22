#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
using namespace std;

static const int CSIZE = 3;
static const int PORT = 8888;

int main() {
    int server_sock;
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0) < 0) {
        perror("ServerSocket Creation Failed\n");
        return -1;
    }
    if ((bind(server_sock, (const sockaddr*)&server_addr, sizeof(server_addr))) < 0) {
        perror("Port Bind Failed\n");
        return -1;
    }
    if ((listen(server_sock, CSIZE)) < 0) {
        perror("Lisen Failed\n");
        return -1;
    }
    pollfd fds[CSIZE];
    sockaddr client_addr;
    socklen_t client_addr_len;
    for (int i = 0; i < CSIZE; i++) {
        fds[i] = accept(server_sock, &client_addr, &client_addr_len);
        fds[i].event = POLLIN;
    }
    return 0;
}
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

static const char* ADDR = "127.0.0.1";
static const int PORT = 8888;

int main() {
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ADDR, &(server_addr.sin_addr));
    server_addr.sin_port = htons(PORT);
    int con[5];
    for (int i = 0; i < 5; i++) {
        con[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (con[i] < 0) {
            perror("Socket Create Failed");
            return -1;
        }
        int ret = connect(con[i], reinterpret_cast<const sockaddr*>(&server_addr), sizeof(server_addr));
        if (ret < 0) {
            perror("Connection Failed");
            return -1;
        }
    }
    // Send and Recieve
    char buffer[1024];
    for (int i = 0; i < 5; i++) {
        int size = sprintf(buffer, "Message From Client %d", i);
        send(con[i], buffer, size, 0);
        size = recv(con[i], buffer, 1024, 0);
        buffer[size] = '\0';
        printf("Recieve: %s\n", buffer);
    }
    return 0;
}
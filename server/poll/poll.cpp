#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>
using namespace std;

static const int PORT = 8888;

int main() {
    int server_sock;
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ServerSocket Creation Failed");
        return -1;
    }
    int reuse = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse)) == -1) {
        perror("Set SO_REUSEADDR Failed");
    }
    if ((bind(server_sock, (const sockaddr*)&server_addr, sizeof(server_addr))) < 0) {
        perror("Port Bind Failed");
        return -1;
    }
    if ((listen(server_sock, 4)) < 0) {
        perror("Lisen Failed");
        return -1;
    }
    // First element is server socket.
    pollfd fds[100];
    int size = 1;
    fds[0].fd = server_sock;
    fds[0].events = POLLIN;

    // About connection
    sockaddr client_addr;
    socklen_t client_addr_len;
    char buffer[1024];

    // main loop
    while (true) {
        // Should check return value for errors occurred.
        poll(fds, size, -1);
        for (int i = 1; i < size; i++) {
            if (fds[i].revents & POLLIN) {
                fds[i].revents = 0;
                int code = 0;
                ioctl(fds[i].fd, FIONREAD, &code);
                if (code != 0) {
                    // Recieve Data
                    int len = read(fds[i].fd, buffer, 1024);
                    buffer[len] = '\0';
                    cout << "MSG FROM " << fds[i].fd << endl;
                    cout << buffer << endl;
                } else {
                    close(fds[i].fd);
                    cout << "Socket Close #" << fds[i].fd << endl;
                    fds[i] = fds[size-1]; size--;
                }
            } 
        }
        if (fds[0].revents & POLLIN) {
            // New Connection Here
            fds[0].revents = 0;
            int new_fd = accept(server_sock, &client_addr, &client_addr_len);
            if (new_fd == -1) {
                perror("Accept Connection Failed");
            } else {
                cout << "Accept Connection #" << new_fd << endl;
                char addr[32] = {0};
                inet_ntop(AF_INET, (void*)&(((sockaddr_in*)(&client_addr))->sin_addr), addr, 32);
                cout << addr << ":" << ntohs(((sockaddr_in*)(&client_addr))->sin_port) << endl;
                fds[size].fd = new_fd;
                fds[size].events = POLLIN;
                size++;
            }
        }
    }
    close(server_sock);
    return 0;
}
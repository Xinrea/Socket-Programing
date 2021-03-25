#include <iostream>
#include <random>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
using namespace std;

static const int PORT = 8888;

int main() {
    // Generate Server Process ID
    unsigned server_id = getpid();
    printf("ServerID: %d\n", server_id);
    // Prepare Server Socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Server Socket Create Failed");
        return -1;
    }
    int check = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &check, sizeof(check)) < 0) {
        perror("Set SO_REUSEPORT Failed");
        return -1;
    }

    // Bind Adress and Port.
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, reinterpret_cast<const sockaddr*>(&server_address), sizeof(server_address)) < 0) {
        perror("Bind Port Failed");
        return -1;
    }

    // Start Listen
    if (listen(server_socket, 3) < 0) {
        perror("Listen Failed");
        return -1;
    }

    // Epoll Prepare
    epoll_event events[5];
    epoll_event ev;
    ev.data.fd = server_socket;
    ev.events = EPOLLIN;
    int epfd = epoll_create(1);
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &ev);

    // Main Loop
    sockaddr client_addr;
    socklen_t client_len;
    char buffer[1024];
    while (true) {
        int wake_num = epoll_wait(epfd, events, 5, -1);
        for (int i = 0; i < wake_num; i++) {
            int fd_now = events[i].data.fd;
            if (fd_now == server_socket) {
                // Accept Connection
                int fd_new = accept(server_socket, &client_addr, &client_len);
                if (fd_new < 0) {
                    perror("Accept Failed");
                } else {
                    ev.data.fd = fd_new;
                    ev.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, fd_new, &ev);
                }
            } else {
                int len = read(fd_now, buffer, 1024);
                if (len == 0) {
                    // Socket Closed
                    ev.data.fd = fd_now;
                    ev.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd_now, nullptr);
                } else {
                    // Read and Response
                    buffer[len] = '\0';
                    printf("Recived: %s\n", buffer);
                    int size = sprintf(buffer, "Ack From [%d]", server_id);
                    send(fd_now, buffer, size, 0);
                }
            }
        }
    }
    return 0;
}
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <unistd.h>
using namespace std;

static const int PORT = 8888;

int main() {
    int server_sock;
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Server Socket Failed");
        return -1;
    }
    int reuse = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("Set SO_REUSEADDR Failed");
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_sock, (const sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind Port Failed");
        return -1;
    }
    if (listen(server_sock,4) < 0) {
        perror("Listen Socket Failed");
        return -1;
    }
    // Something for new connection and epoll
    sockaddr client_addr;
    socklen_t client_len;
    char buffer[1024];
    int epfd = epoll_create(1);
    // Add server socket in management
    epoll_event ev;
    ev.data.fd = server_sock;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &ev);

    epoll_event events[5];
    // main loop
    while (true) {
        int n = epoll_wait(epfd, events, 5, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_sock) {
                // New connection
                int new_fd = accept(server_sock, &client_addr, &client_len);
                if (new_fd < 0) {
                    perror("Accept Failed");
                } else {
                    printf("Accept Connection #%d\n", new_fd);
                    ev.data.fd = new_fd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &ev);
                }
            } else {
                int len = 0;
                ioctl(events[i].data.fd, FIONREAD, &len);
                if (len == 0) {
                    // Which means socket closed.
                    close(events[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
                    printf("Close #%d\n", events[i].data.fd);
                } else {
                    len = read(events[i].data.fd, buffer, 1024);
                    buffer[len] = '\0';
                    printf("MSG FROM %d\n%s\n", events[i].data.fd, buffer);
                }
            }
        }
    }
    return 0;
}
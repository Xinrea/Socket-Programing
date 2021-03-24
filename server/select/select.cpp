/*
    This simple select-server accept CSIZE clients, read and print data recvied from client. When all connections come to end, program ends.
    Need to manually manage client socket states, for managing "rset", heart of select.
*/

#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
using namespace std;

static const int PORT = 8888;
static const int CSIZE = 3;

int main() {
    int serv_sock;
    if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket Creation Failed" << endl;
        return -1;
    }
    sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (bind(serv_sock, (const sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cout << "Bind Port Failed" << endl;
        return -1;
    }
    if (listen(serv_sock, 4) < 0) {
        cout << "Listen Failed" << endl;
        return -1;
    }
    sockaddr_in client_addr;
    socklen_t addr_len;
    char* buffer = new char[1024];
    // Preserve postion 0 for listening socket.
    int cfd[CSIZE+2]; cfd[0] = serv_sock;
    int f_end = 0;
    int m_fd = serv_sock;
    fd_set rset;
    while (true) {
        FD_ZERO(&rset);
        cout << "Listen Sockets:" << endl;
        for (int i = 0; i <= f_end; i++) {
            FD_SET(cfd[i], &rset);
            cout << cfd[i] << " ";
        }
        cout << endl;
        select(m_fd+1, &rset, NULL, NULL, NULL);
        for (int i = 0; i <= f_end; i++) {
            if (FD_ISSET(cfd[i],&rset)) {
                // serv_socket accept connection.
                if (i == 0) {
                    if ((cfd[f_end+1] = accept(serv_sock, (sockaddr*)&client_addr, &addr_len)) < 0) {
                        cout << "Accept Failed" << endl;
                        return -1;
                    }
                    cout << "New Connection Socket: " << cfd[f_end+1] << "; Total: " << f_end+1 << endl;
                    if (cfd[f_end+1] > m_fd) m_fd = cfd[f_end+1];
                    f_end++;
                    continue;
                }
                int code = 0;
                ioctl(cfd[i], FIONREAD, &code);
                if (code == 0) {
                    cout << cfd[i] << " closed" << endl;
                    cfd[i] = cfd[f_end];
                    f_end--;
                } else {
                    int len = read(cfd[i], buffer, 1024);
                    buffer[len] = '\0';
                    cout << "MSG FROM " << cfd[i] << endl << buffer << endl;
                }
            }
        }
    }
    cout << "All Connection Ended" << endl;
    delete buffer;
    close(serv_sock);
    return 0;
}
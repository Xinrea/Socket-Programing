#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
using namespace std;

static const int SERVER_PORT = 8888;
static const string SERVER_ADDRESS = "127.0.0.1";
static const int CONNECT_NUM = 3;

int main() {
  int sockfd[CONNECT_NUM];
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  if (inet_pton(AF_INET, SERVER_ADDRESS.c_str(), &server_addr.sin_addr) != 1) {
    cout << "ServerAdress Convertion Failed" << endl;
    return -1;
  }
  for (int i = 0; i < CONNECT_NUM; i++) {
    if ((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      cout << "Socket Creation Failed" << endl;
      return -1;
    }
    if (connect(sockfd[i], (const sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
      cout << "Connection Failed" << endl;
      return -1;
    }
  }
  string msg = "hello from client";
  int send_count = 3;
  while (send_count--) {
    for (int i = 0; i < CONNECT_NUM; i++) {
      string msg_c = msg + to_string(i);
      //cout << "MSG SEND" << endl << msg_c << endl;
      send(sockfd[i], msg_c.c_str(), msg_c.length(), 0);
      sleep(1);
    }
  }
  for (int i = 0; i < CONNECT_NUM; i++) {
    close(sockfd[i]);
  }
  return 0;
}

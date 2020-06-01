#pragma once

#include <sys/socket.h> /* socket, connect */

class SocketGuard {
public:
  SocketGuard() : sockfd(-1){};
  SocketGuard(const SocketGuard &) = delete;
  bool connect() {
    if (sockfd != -1) {
      std::cerr << "Socket already used";
      return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
      return false;
    return true;
  }
  int get_sockfd() const { return sockfd; }
  ~SocketGuard() {
    if (sockfd != -1)
      close(sockfd);
    sockfd = -1;
  }

private:
  int sockfd;
};
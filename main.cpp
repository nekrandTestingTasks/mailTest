#include <fstream>
#include <iostream>
#include <optional>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include "SocketGuard.h"

void parseURL(const std::string &input, std::string &host, std::string &path) {
  auto dist = input.find('/');
  if (dist == std::string::npos) {
    path = '/';
    host = input;
    return;
  }
  host = input.substr(0, dist);
  path = input.substr(dist, std::string::npos);
}

bool buildRequest(int argc, char *argv[], std::string &host,
                  std::string &message) {
  if (argc < 2) {
    std::cout << "Address not set";
    return false;
  }

  if (argc > 2) {
    std::cout << "Only address parameter expected";
    return false;
  }
  std::string path;
  parseURL(argv[1], host, path);

  std::cout << "h: " << host << " p: " << path;
  message = "GET " + path + " HTTP/1.0\r\n\r\n";
  return true;
}

bool sendRequest(const SocketGuard &soc, const std::string &msg) {
  int total = msg.size();
  int sent = 0;
  int bytes;
  do {
    bytes = write(soc.get_sockfd(), msg.c_str() + sent, total - sent);
    if (bytes < 0) {
      std::cerr << "ERROR writing message to socket";
      return false;
    }

    if (bytes == 0)
      break;
    sent += bytes;
  } while (sent < total);

  return true;
}

void saveResponse(const SocketGuard &soc) {
  const int kResponsePieceSize = 4096;

  std::ofstream out("output.txt");
  if (!out) {
    std::cerr << "Cannot open output file";
    return;
  }
  int bytes;
  char response[kResponsePieceSize];
  std::fill(response, response + kResponsePieceSize, '\0');
  do {
    bytes = read(soc.get_sockfd(), response, kResponsePieceSize);
    if (bytes < 0) {
      std::cerr << "ERROR reading response from socket";
      return;
    }
    out.write(response, bytes);
  } while (bytes != 0);
}

int main(int argc, char *argv[]) {
  const int kPort = 80;
  std::string host;
  std::string message;
  if (!buildRequest(argc, argv, host, message))
    return 0;

  struct hostent *server;
  struct sockaddr_in serv_addr;

  SocketGuard socket;
  if (!socket.connect()) {
    std::cerr << "ERROR opening socket";
    return 0;
  }

  server = gethostbyname(host.c_str());
  if (server == NULL) {
    std::cerr << "ERROR, no such host";
    return 0;
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(kPort);
  memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  if (connect(socket.get_sockfd(), (struct sockaddr *)&serv_addr,
              sizeof(serv_addr)) < 0) {
    std::cerr << "ERROR connecting";
    return 0;
  }

  if (!sendRequest(socket, message))
    return 0;

  saveResponse(socket);

  return 0;
}
#include "tcp_server.h"
#include "../kv/kvstore.h"
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>

namespace kv {

    TCPServer::TCPServer(int port, int num_shards) : port_(port), store_(num_shards), server_sock_(-1) {
        server_sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock_ < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        //Now bind the socket to the port

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);

        if (bind(server_sock_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(server_sock_);
            throw std::runtime_error("Failed to bind socket");
        }

        //Now listen for incoming connections

        if (listen(server_sock_, SOMAXCONN) < 0) {
            close(server_sock_);
            throw std::runtime_error("Failed to listen on socket");
        }
        std::cout << "Server listening on port " << port_ << std::endl;

    }

}
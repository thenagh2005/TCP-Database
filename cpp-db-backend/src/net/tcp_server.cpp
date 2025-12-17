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

    TCPServer::TCPServer(int port, int num_shards) : port_(port), store_(num_shards), server_sock_(-1), running_(false) {
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

    void TCPServer::start() { //Start the server
        running_ = true;
        std::cout << "Server started, waiting for connections..." << std::endl;
        while (running_) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client_sock = accept(server_sock_, (struct sockaddr*)&client_addr, &client_len); //Accept an incoming connection

            if (client_sock < 0) {
                //SOMETHING WENT WRONG AAAAHHHHH
                if (running_) {
                    std::cerr << "Failed to accept connection" << std::endl;
                }
                
                continue;
            }

            //Now do stuff with the connection

            std::cout << "Accepted connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
            threads_.emplace_back(&TCPServer::handle_client, this, client_sock); //Handle the client in a new thread

            


        }
    }

    void TCPServer::stop() {
        running_ = false;
        close(server_sock_);
        server_sock_ = -1;

        for (auto &t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }

        threads_.clear();
        std::cout << "Server stopped." << std::endl;
    }

    TCPServer::~TCPServer() {
        if (server_sock_ >= 0) {
            stop();

            //I will close server sock in stop() function
        }
    }

}
#include "tcp_server.h"
#include "../kv/kvstore.h"
#include "../kv/zset.h"
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <vector>



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

    std::string TCPServer::process_command(const std::string &cmd) {
        std::istringstream iss(cmd);
        std::string token;
        std::vector<std::string> tokens;

        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) {
            return "ERROR: Empty command\n";
        }

        std::string command = tokens[0];

        if (command == "SET") {
            if (tokens.size() != 3) {
                return "ERROR: SET command requires 2 arguments\n";
            }

            store_.set(tokens[1], tokens[2]);
            return "OK\n";
        } else if (command == "GET") {
            if (tokens.size() != 2) {
                return "ERROR: GET command requires 1 argument\n";
            }

            auto value = store_.get(tokens[1]);
            if (value) {
                return *value + "\n";
            } else {
                return "It ain't there pal\n";
            }

        } else if (command == "DELETE") {
            if (tokens.size() != 2) {
                return "ERROR: DELETE command requires 1 argument\n";
            }

            bool deleted = store_.del(tokens[1]);
            if (deleted) {
                return "OK\n";
            } else {
                return "Key not found\n";
            }

        } else if (command == "EXISTS") {
            if (tokens.size() != 2) {
                return "ERROR: EXISTS command requires 1 argument\n";
            }

            bool exists = store_.exists(tokens[1]);
            return exists ? "1\n" : "0\n";
        } else if (command == "ALL") {
            if (tokens.size() != 1) {
                return "ERROR: ALL command takes no arguments\n";
            }

            auto entries = store_.all_entries();
            std::string response;
            for (const auto &[key, value] : entries) {
                response += key + " " + value + "\n";
            }

            return response.empty() ? "nil\n" : response;
        } 
         else if (command == "ZADD") {
            if (tokens.size() != 4) {
                return "ERROR: ZADD command requires 3 arguments\n";
            }

            const std::string &key = tokens[1];
            double score;

            try {
                score = std::stod(tokens[2]);
            } catch (const std::invalid_argument &) {
                return "ERROR: Score must be a valid number\n";
            }

            const std::string &member = tokens[3];
            bool added = store_.zadd(key, member, score);
            return added ? "1\n" : "0\n";


            
        } else if (command == "ZREM") {
            if (tokens.size() != 3) {
                return "ERROR: ZREM command requires 2 arguments\n";
            }

            const std::string &key = tokens[1];
            const std::string &member = tokens[2];
            bool removed = store_.zrem(key, member);
            return removed ? "1\n" : "0\n";
            
        } else if (command == "ZSCORE") {
            if (tokens.size() != 3) {
                return "ERROR: ZSCORE command requires 2 arguments\n";
            }

            const std::string &key = tokens[1];
            const std::string &member = tokens[2];
            auto score = store_.zscore(key, member);
            if (score) {
                return std::to_string(*score) + "\n";
            } else {
                return "nil\n";
            }
            
        } else if (command == "ZRANK") {
            if (tokens.size() != 3) {
                return "ERROR: ZRANK command requires 2 arguments\n";
            }

            const std::string &key = tokens[1];
            const std::string &member = tokens[2];
            auto rank = store_.zrank(key, member);
            if (rank) {
                return std::to_string(*rank) + "\n";
            } else {
                return "nil\n";
            }
            
        } else if (command == "ZRANGE") {
            if (tokens.size() != 4) {
                return "ERROR: ZRANGE command requires 3 arguments\n";
            }

            const std::string &key = tokens[1];
            int start, stop;

            try {
                start = std::stoi(tokens[2]);
                stop = std::stoi(tokens[3]);
            } catch (const std::invalid_argument &) {
                return "ERROR: Start and stop must be valid integers\n";
            }

            auto range = store_.zrange(key, start, stop);
            std::string response;
            for (const auto &[member, score] : range) {
                response += member + " " + std::to_string(score) + "\n";
            }
            return response.empty() ? "nil\n" : response;
            

            
        } else if (command == "ZSIZE") {
            if (tokens.size() != 2) {
                return "ERROR: ZSIZE command requires 1 argument\n";
            }

            const std::string &key = tokens[1];
            size_t size = store_.zsize(key);
            return std::to_string(size) + "\n";
            
        } else {
            return "ERROR: Unknown command\n";
        }
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

    void TCPServer::handle_client(int client_sock) {
        //Handle el cliente here

        char buffer[1024];

        while (running_) {
            memset(buffer, 0, sizeof(buffer));

            ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer)-1, 0);

            if (bytes_read <= 0) {
                //Something went wrong or client disconnected
                break;
            }

            std::string command(buffer, bytes_read);
            std::string response = process_command(command);

            send(client_sock, response.c_str(), response.size(), 0);
        }

        close(client_sock);
        std::cout << "Client disconnected." << std::endl;


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
        
        stop();

            //I will close server sock in stop() function
        
    }

}
#include "net/tcp_server.h"
#include <iostream>
#include <csignal>

kv::TCPServer* server_ptr = nullptr;

void signal_handler(int signal) {
    if (server_ptr) {
        std::cout << "\nShutting down server..." << std::endl;
        server_ptr->stop();
        exit(0);
    }
}

int main() {
    try {
        kv::TCPServer server(8080);
        server_ptr = &server;
        
        // Handle Ctrl+C gracefully
        std::signal(SIGINT, signal_handler);
        
        std::cout << "Starting TCP server on port 8080..." << std::endl;
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
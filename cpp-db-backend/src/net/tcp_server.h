#pragma once
#include "../kv/kvstore.h"
#include <string>
#include <thread>
#include <vector>
#include <atomic>

namespace kv {
    class TCPServer {
        public:
            TCPServer(int port, int num_shards = 16);
            ~TCPServer();

            void start();
            void stop();
        
        private:
            void handle_client(int client_sock);
            std::string process_command(const std::string &cmdline);

            KVStore store_;
            int port_;
            int server_sock_;
            std::atomic<bool> running_;
            std::vector<std::thread> threads_;
    };
}
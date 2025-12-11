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

    }

}
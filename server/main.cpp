#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <csignal>

#include "Server.hpp"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: server <port>\n");
        return 1;
    }

    int port = atoi(argv[1]);

    Server server(port);
    server.run();
    
    return 0;
}
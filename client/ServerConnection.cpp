#include "ServerConnection.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <istream>

class ServerConnection {
    private:
        int socket_fd;

    public:
        ServerConnection(std::string ipAddr, int port);
        int get(std::string filename);
        int upload(std::string filename);
};
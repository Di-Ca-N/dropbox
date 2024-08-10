#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

#include "handlers/client-handler.hpp"
#include "handlers/server-handler.hpp"

int convertCharArrayToPort(char *array);

void acceptConnections(int port, void (*handler)(int));
int createSocket(int port);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: binding-agent <client-port> <server-port>\n";
        return 1;
    }

    int clientPort, serverPort;
    try {
        clientPort = convertCharArrayToPort(argv[1]);
        serverPort = convertCharArrayToPort(argv[2]);
    } catch (std::invalid_argument) {
        std::cerr << "Received invalid arguments\n";
        return 1;
    }

    try {
        std::thread clientThread(acceptConnections, clientPort, handleClientConnection);
        std::thread serverThread(acceptConnections, serverPort, handleServerConnection);

        clientThread.join();
        serverThread.join();
    } catch (std::runtime_error const &e) {
        std::cerr << e.what();
        return 1;
    }

    return 0;
}

void acceptConnections(int port, void (*handler)(int)) {
    int socketDescriptor = createSocket(port);
    std::vector<std::thread> connections;

    while (true) {
        int remoteDescriptor = accept(socketDescriptor, nullptr, nullptr);
        connections.push_back(std::thread(handler, remoteDescriptor));
    }

    for (auto &connection : connections) {
        connection.join();
    }
}

int createSocket(int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        throw std::runtime_error("Couldn't create socket\n");
    }
    int optVal = 1;
    setsockopt(sock_fd, 1, SO_REUSEADDR, &optVal, sizeof(optVal));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int err;
    err = bind(sock_fd, (sockaddr*) &addr, sizeof(addr));

    if (err == -1) {
        throw std::runtime_error("Couldn't bind socket\n");
    }

    err = listen(sock_fd, 10);
    if (err == -1) {
        throw std::runtime_error("Couldn't make socket listen to port\n");
    }

    return sock_fd;
}

int convertCharArrayToPort(char *array) {
    int port = atoi(array);

    if (port == 0)
        throw std::invalid_argument("Char array does not correspond to a valid port");

    return port;
}


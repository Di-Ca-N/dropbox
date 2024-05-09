#include "Server.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "Messages.h"

Server::Server(int port) {
    this->port = port;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        throw std::runtime_error("Error on socket creation");
    }

    int optVal = 1;
    setsockopt(sock_fd, 1, SO_REUSEADDR, &optVal, sizeof(optVal));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock_fd, (sockaddr*) &addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Error on socket bind");
    }
}

Server::~Server() {
    std::cout << "Finishing server...";
    for (auto &thread : openConnections) {
        thread.join();
    }
    close(sock_fd);
    std::cout << "Done!\n";
}

void Server::run() {
    if (listen(sock_fd, 10) == -1) {
        throw std::runtime_error("Error on socket listen");
    }
    std::cout << "Server listening on port " << port << "\n";

    running = true;

    while (running) {
        int clientSocket = accept(sock_fd, nullptr, nullptr);
        openConnections.push_back(std::thread([&,this]{ handleClient(clientSocket); }));
    }
}

void Server::handleClient(int client_socket) {
    while (true) {
        Message msg;
        receiveMessage(client_socket, &msg);
        printMsg(&msg);
    }
}
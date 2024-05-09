#include "Server.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <filesystem>

#include "Messages.h"
#include "controllers/UploadController.hpp"
#include "controllers/DownloadController.hpp"

#define DATA_DIR "data"

Server::Server(int port) {
    this->port = port;

    this->sock_fd = socket(AF_INET, SOCK_STREAM, 0);

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

void Server::stop() {
    running = false;
}


void handleClient(int clientSocket) {
    try {
        while (true) {
            std::string username = receiveAuth(clientSocket);
            std::cout << username << "\n";
            sendOk(clientSocket);

            Message msg = receiveMessage(clientSocket);
            
            switch (msg.type) {
                case MsgType::MSG_UPLOAD:
                    UploadController(clientSocket, username).run(); 
                    break;
                case MsgType::MSG_DOWNLOAD:
                    DownloadController(clientSocket, username).run(); 
                    break;
                default:
                    std::cout << "Unkown command\n";
                    break;
            }
        }
    } catch (BrokenPipe) {
        std::cout << "Client disconnected\n";
    }
    close(clientSocket);
}


void Server::run() {
    std::filesystem::path dataDir = std::filesystem::current_path() / DATA_DIR;
    std::filesystem::create_directory(dataDir);
    std::filesystem::current_path(dataDir);


    if (listen(sock_fd, 10) == -1) {
        throw std::runtime_error("Error on socket listen");
    }
    std::cout << "Server listening on port " << port << "\n";

    running = true;

    while (running) {
        int clientSocket = accept(sock_fd, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cout << "Error on client connection\n";
            break;
        }
    
        openConnections.push_back(std::thread(handleClient, clientSocket));
    }
}

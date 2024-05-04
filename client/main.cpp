#include <iostream>
#include <memory>
#include <string>

#include "Client.hpp"

int main(int argc, char* argv[]) {
    int port;

    if (argc != 4) {
        std::cerr << "usage: ./client <username> <ip> <port>" << std::endl;
        exit(1);
    }

    try {
        port = std::stoi(argv[3]);
    } catch(const std::invalid_argument& e) {
        std::cerr << "Cannot recognize port" << std::endl;
        exit(1);
    }

    std::unique_ptr<Client> client = std::make_unique<Client>(Client());
    client->run(argv[1], argv[2], port);
    return 0;
}

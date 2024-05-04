#include <memory>
#include <iostream>

#include "CLI.hpp"
#include "ClientState.hpp"

CLI::CLI() {
    this->clientState = std::make_shared<ClientState>(ClientState::STATE_ACTIVE);
}

void CLI::run(std::string username, std::string ip, int port) {
    std::string command;
    while (*clientState == ClientState::STATE_ACTIVE) {
        std::cout << "> ";
        std::getline(std::cin, command); 

        if (command.compare("exit") == 0)
            *clientState = ClientState::STATE_CLOSING;
        else
            std::cerr << "Command not recognized" << std::endl;
    }
}


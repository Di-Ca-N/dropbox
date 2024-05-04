#include <memory>
#include <iostream>

#include "CLI.hpp"
#include "ClientState.hpp"
#include "CLICallback.hpp"
#include "Connection.hpp"

CLI::CLI(std::shared_ptr<ClientState> clientState,
             std::shared_ptr<Connection> connection) {
    this->clientState = std::move(clientState);
    this->callback = std::make_unique<CLICallback>(CLICallback(
                                                      std::move(connection)));
}

void CLI::run() {
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

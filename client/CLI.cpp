#include <memory>
#include <iostream>
#include <filesystem>

#include "CLI.hpp"
#include "ClientState.hpp"
#include "ServerMonitor.hpp"
#include "ClientMonitor.hpp"

#define SYNC_DIR "sync_dir"

void CLI::run(std::string username, std::string ip, int port) {
    connection = std::make_shared<Connection>(Connection());
    connection->connectToServer(username, ip, port);

    clientState = std::make_shared<ClientState>(ClientState::STATE_ACTIVE);

    getSyncDir();

    std::string command;
    while (*clientState == ClientState::STATE_ACTIVE) {
        std::cout << "> ";
        std::getline(std::cin, command); 

        if (command.compare("exit") == 0)
            *clientState = ClientState::STATE_CLOSING;
        else if (command.compare("get_sync_dir") == 0)
            getSyncDir();
        else
            std::cerr << "Command not recognized" << std::endl;
    }

    serverThread.join();
    clientThread.join();
}

void CLI::getSyncDir() {
    if (!std::filesystem::exists(SYNC_DIR)
            || !std::filesystem::is_directory(SYNC_DIR))
        std::filesystem::create_directory(SYNC_DIR);

    if (!serverThread.joinable()) {
        serverMonitor = std::make_unique<ServerMonitor>(ServerMonitor(
                                                           clientState, connection));
        serverThread = std::thread(&ServerMonitor::run, std::ref(*serverMonitor));
    }

    if (!clientThread.joinable()) {
        clientMonitor = std::make_unique<ClientMonitor>(ClientMonitor(
                                                           clientState, connection));
        clientThread = std::thread(&ClientMonitor::run, std::ref(*clientMonitor));
    }
}


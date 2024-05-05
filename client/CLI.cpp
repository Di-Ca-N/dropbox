#include <memory>
#include <iostream>
#include <filesystem>
#include <vector>
#include <unistd.h>

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

    std::string line;
    std::string token;
    std::istringstream iss;
    std::vector<std::string> tokens;
    while (*clientState == ClientState::STATE_ACTIVE) {
        tokens.clear();

        std::cout << "> ";
        std::getline(std::cin, line);
        iss = std::istringstream(line);
        while (iss >> token)
            tokens.push_back(token);

        if (tokens.size() == 1 && tokens[0].compare("exit") == 0)
            *clientState = ClientState::STATE_CLOSING;
        else if (tokens.size() == 1 && tokens[0].compare("get_sync_dir") == 0)
            getSyncDir();
        else if (tokens.size() == 2 && tokens[0].compare("upload") == 0)
            connection->upload(tokens[1]);
        else if (tokens.size() == 2 && tokens[0].compare("download") == 0)
            connection->download(tokens[1]);
        else if (tokens.size() == 2 && tokens[0].compare("delete") == 0)
            connection->delete_(tokens[1]);
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


#include <memory>
#include <iostream>
#include <sys/stat.h>
#include <poll.h>
#include <unistd.h>

#include "CLI.hpp"
#include "ClientState.hpp"
#include "CommandParser.hpp"
#include "Command.hpp"

#define SYNC_DIR "sync_dir"

void CLI::run(std::string username, std::string ip, int port) {
    makeConnection(username, ip, port);
    startClientState(AppState::STATE_UNTRACKED);

    initializeSyncDir();

    bool nextLine = true;

    struct pollfd cinFd = { .fd = STDIN_FILENO, .events = POLLIN };

    std::string line;
    while (clientState->get() != AppState::STATE_CLOSING) {
        printPromptIfNewCommand(nextLine);

        int cinPoll = poll(&cinFd, 1, 0);

        if (cinPoll > 0 && cinFd.revents & POLLIN) {
            std::getline(std::cin, line);
            try {
                std::unique_ptr<Command> command = CommandParser::parse(
                        line,
                        SYNC_DIR,
                        weak_from_this(),
                        std::weak_ptr(clientState),
                        std::weak_ptr(connection)
                );
                command->execute();
            } catch(const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            nextLine = true;
        }
    }

    serverThread.join();
    clientThread.join();
}

void CLI::makeConnection(std::string username, std::string ip, int port) {
    connection = std::make_shared<Connection>(Connection());
    connection->connectToServer(username, ip, port);
}

void CLI::startClientState(AppState state) {
    clientState = std::make_shared<ClientState>(state);
}

void CLI::printPromptIfNewCommand(bool &nextLine) {
    if (nextLine) {
        std::cout << "> ";
        std::cout.flush();
        nextLine = false;
    }
}

void CLI::initializeSyncDir() {
    GetSyncDir cmd = GetSyncDir(
            SYNC_DIR,
            weak_from_this(),
            std::weak_ptr(clientState),
            std::weak_ptr(connection)
    );
    cmd.execute();
}

void CLI::restartServerThread() {
    if (serverThread.joinable())
        serverThread.join();

    serverMonitor = std::make_unique<ServerMonitor>(
            ServerMonitor(clientState, connection)
    );

    serverThread = std::thread(
            &ServerMonitor::run,
            std::ref(*serverMonitor)
    );
}

void CLI::restartClientThread() {
    if (clientThread.joinable())
        clientThread.join();

    clientMonitor = std::make_unique<ClientMonitor>(
            ClientMonitor(clientState, connection)
    );

    clientThread = std::thread(
            &ClientMonitor::run,
            std::ref(*clientMonitor),
            SYNC_DIR
    );
}

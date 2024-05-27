#include <memory>
#include <iostream>
#include <sys/stat.h>
#include <poll.h>
#include <unistd.h>

#include "CLI.hpp"
#include "ClientState.hpp"
#include "Command.hpp"

#define SYNC_DIR "sync_dir"

void CLI::run(std::string username, std::string ip, int port) {
    bool newLine;
    struct pollfd cinFd;

    makeConnection(username, ip, port);
    startClientState(AppState::STATE_UNTRACKED);
    initializeCommandParser();
    initializeSyncDir();

    newLine = true;
    cinFd = { .fd = STDIN_FILENO, .events = POLLIN };

    while (clientState->get() != AppState::STATE_CLOSING) {
        if (newLine) {
            printPrompt();
            newLine = false;
        }
        
        if (newCommand(&cinFd)) {
            parseCommand(newLine);
            newLine = true;
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

void CLI::printPrompt() {
    std::cout << "> ";
    std::cout.flush();
}

void CLI::initializeCommandParser() {
    commandParser = std::make_unique<CommandParser>(
            CommandParser(
                SYNC_DIR,
                weak_from_this(),
                std::weak_ptr(clientState),
                std::weak_ptr(connection)
            )
    );
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

void CLI::parseCommand(bool &newLine) {
    std::string line;

    std::getline(std::cin, line);
    try {
        std::unique_ptr<Command> command = commandParser->parse(line);
        command->execute();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

bool CLI::newCommand(struct pollfd *cinFd) {
    int cinPoll = poll(cinFd, 1, 0);
    return (cinPoll > 0) && (cinFd->revents & POLLIN);
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

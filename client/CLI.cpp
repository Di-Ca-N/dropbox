#include <memory>
#include <iostream>
#include <sys/stat.h>
#include <poll.h>
#include <unistd.h>

#include "CLI.hpp"
#include "ClientState.hpp"
#include "Command.hpp"
#include "ClientConfig.hpp"

void CLI::run(std::string username, std::string ip, int port) {
    bool newLine;
    struct pollfd cinFd;

    makeConnection(username, ip, port);
    makeHistory();
    startClientState(AppState::STATE_UNTRACKED);
    initializeHeartbeatMonitor();
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

    heartbeatThread.join();
    serverThread.join();
    clientThread.join();
}

void CLI::makeConnection(std::string username, std::string ip, int port) {
    connection = std::make_shared<Connection>(Connection());
    
    while (true) {
        try {
            connection->connectToService(username, ip, port);
        } catch (BinderConnectionError) {
            continue;
        } catch (...) {}

        break;
    }
}

void CLI::makeHistory() {
    eventHistory = std::make_shared<EventHistory>();
}

void CLI::startClientState(AppState state) {
    clientState = std::make_shared<ClientState>(state);
}

void CLI::printPrompt() {
    std::cout << "> ";
    std::cout.flush();
}

void CLI::initializeHeartbeatMonitor() {
    heartbeatMonitor = std::make_unique<HeartbeatMonitor>(
            HeartbeatMonitor(
                clientState,
                connection
            )
    );

    heartbeatThread = std::thread(
            &HeartbeatMonitor::run,
            std::ref(*heartbeatMonitor)
    );
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
    try {
        cmd.execute();
    } catch (ServerConnectionError) {
        std::cout << "Service is offline. Please, try again later.\n";
    } catch (BrokenPipe) {
        std::cout << "Service is offline. Please, try again later.\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }
}

void CLI::parseCommand(bool &newLine) {
    std::string line;

    std::getline(std::cin, line);
    try {
        std::unique_ptr<Command> command = commandParser->parse(line);
        command->execute();
    } catch (NoCommandException &e) {
        std::cout << e.what() << "\n";
    } catch (InvalidCommandException &e) {
        std::cout << e.what() << "\n";
    } catch (InvalidArgumentException &e) {
        std::cout << e.what() << "\n";
    } catch (ServerConnectionError) {
        std::cout << "Service is offline. Please, try again later.\n";
    } catch (BrokenPipe) {
        std::cout << "Service is offline. Please, try again later.\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
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
            ServerMonitor(clientState, connection, eventHistory)
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
            ClientMonitor(clientState, connection, eventHistory)
    );

    clientThread = std::thread(
            &ClientMonitor::run,
            std::ref(*clientMonitor),
            SYNC_DIR
    );
}

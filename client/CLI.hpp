#ifndef CLI_H
#define CLI_H

#include <memory>
#include <thread>

#include "ClientState.hpp"
#include "Connection.hpp"
#include "ServerMonitor.hpp"
#include "CommandParser.hpp"
#include "ThreadOwner.hpp"
#include "ClientMonitor.hpp"

class CLI : public ThreadOwner, public std::enable_shared_from_this<CLI> {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;
    std::shared_ptr<EventHistory> eventHistory;
    std::unique_ptr<ServerMonitor> serverMonitor;
    std::unique_ptr<ClientMonitor> clientMonitor;
    std::unique_ptr<CommandParser> commandParser;
    std::thread serverThread;
    std::thread clientThread;

    void makeConnection(std::string username, std::string ip, int port);
    void makeHistory();
    void startClientState(AppState state);
    void printPrompt();
    void initializeCommandParser();
    void initializeSyncDir();
    void parseCommand(bool &newLine);
    bool newCommand(struct pollfd *cinFd);

public:
    void run(std::string username, std::string ip, int port);
    void restartServerThread() override;
    void restartClientThread() override;
};

#endif

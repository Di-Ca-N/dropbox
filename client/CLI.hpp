#ifndef CLI_H
#define CLI_H

#include <memory>
#include <thread>

#include "ClientState.hpp"
#include "Connection.hpp"
#include "ServerMonitor.hpp"
#include "ThreadOwner.hpp"
#include "ClientMonitor.hpp"

class CLI : public ThreadOwner, public std::enable_shared_from_this<CLI> {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;
    std::unique_ptr<ServerMonitor> serverMonitor;
    std::unique_ptr<ClientMonitor> clientMonitor;
    std::thread serverThread;
    std::thread clientThread;

    void makeConnection(std::string username, std::string ip, int port);
    void startClientState(AppState state);
    void printPromptIfNewCommand(bool &nextLine);
    void initializeSyncDir();

public:
    void run(std::string username, std::string ip, int port);
    void restartServerThread() override;
    void restartClientThread() override;
};

#endif

#ifndef CLI_H
#define CLI_H

#include <memory>
#include <thread>

#include "ClientState.hpp"
#include "Connection.hpp"
#include "ServerMonitor.hpp"
#include "ClientMonitor.hpp"

class CLI {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;
    std::unique_ptr<ServerMonitor> serverMonitor;
    std::unique_ptr<ClientMonitor> clientMonitor;
    std::thread serverThread;
    std::thread clientThread;

    void getSyncDir();
    void listClient();

public:
    void run(std::string username, std::string ip, int port);
};

#endif

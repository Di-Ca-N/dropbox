#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

#include <memory>

#include "ClientState.hpp"
#include "Connection.hpp"

class ServerMonitor {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;

public:
    ServerMonitor(std::shared_ptr<ClientState> clientState,
            std::shared_ptr<Connection> connection);
    void run();
};

#endif

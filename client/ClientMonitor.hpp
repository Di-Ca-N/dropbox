#ifndef CLIENT_MONITOR_H
#define CLIENT_MONITOR_H

#include <memory>
#include <string>

#include "ClientState.hpp"
#include "Connection.hpp"

class ClientMonitor {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;

public:
    ClientMonitor(std::shared_ptr<ClientState> clientState,
            std::shared_ptr<Connection> connection);
    void run(std::string sync_dir);
};

#endif

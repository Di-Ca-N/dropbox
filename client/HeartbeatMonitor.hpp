#ifndef HEARTBEAT_MONITOR_H
#define HEARTBEAT_MONITOR_H

#include <memory>

#include "ClientState.hpp"
#include "Connection.hpp"

class HeartbeatMonitor {
    private:
        std::shared_ptr<ClientState> clientState;
        std::shared_ptr<Connection> connection;

    public:
        HeartbeatMonitor(std::shared_ptr<ClientState> clientState,
                std::shared_ptr<Connection> connection);
        void run();
};

#endif

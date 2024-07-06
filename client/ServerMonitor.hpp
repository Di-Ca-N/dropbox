#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

#include <memory>

#include "ClientState.hpp"
#include "Connection.hpp"
#include "EventHistory.hpp"

class ServerMonitor {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;
    std::shared_ptr<EventHistory> history;

public:
    ServerMonitor(std::shared_ptr<ClientState> clientState,
            std::shared_ptr<Connection> connection,
            std::shared_ptr<EventHistory> history);
    void run();
    void applyTempIfContentUpdate(FileOperation &operation);
};

#endif

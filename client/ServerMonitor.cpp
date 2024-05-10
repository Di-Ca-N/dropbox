#include <memory>
#include <unistd.h>

#include "ServerMonitor.hpp"
#include "ClientState.hpp"
#include "Connection.hpp"

ServerMonitor::ServerMonitor(std::shared_ptr<ClientState> clientState,
        std::shared_ptr<Connection> connection) {
    this->clientState = clientState;
    this->connection = connection;
}

void ServerMonitor::run() {
    while (clientState->get() == AppState::STATE_ACTIVE) {
        connection->syncRead();
    }
}


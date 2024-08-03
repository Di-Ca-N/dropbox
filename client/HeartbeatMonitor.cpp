#include "HeartbeatMonitor.hpp"

#define MAX_WAIT 1

HeartbeatMonitor::HeartbeatMonitor(
        std::shared_ptr<ClientState> clientState,
        std::shared_ptr<Connection> connection) {
    this->clientState = clientState;
    this->connection = connection;
}

void HeartbeatMonitor::run() {
    while (clientState->get() != AppState::STATE_CLOSING) {
        if (!connection->hearsHeartbeat(MAX_WAIT)) {
            connection->retryConnection();
        }
    }
}

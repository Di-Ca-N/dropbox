#include <memory>
#include <unistd.h>

#include "ServerMonitor.hpp"
#include "ClientState.hpp"
#include "Connection.hpp"

ServerMonitor::ServerMonitor(
        std::shared_ptr<ClientState> clientState,
        std::shared_ptr<Connection> connection,
        std::shared_ptr<EventHistory> history) {
    this->clientState = clientState;
    this->connection = connection;
    this->history = history;
}

void ServerMonitor::run() {
    std::optional<FileOperation> operation;

    while (clientState->get() == AppState::STATE_ACTIVE) {
        operation = connection->syncRead();

        if (operation.has_value())
            history->pushEvent(operation.value());
    }
}


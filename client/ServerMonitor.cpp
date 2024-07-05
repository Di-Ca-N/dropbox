#include <memory>
#include <unistd.h>
#include <iostream>

#include "ServerMonitor.hpp"
#include "ClientState.hpp"
#include "Connection.hpp"
#include "Messages.hpp"

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

    try {
        while (clientState->get() == AppState::STATE_ACTIVE) {
            connection->syncRead(history);
        }
    } catch (BrokenPipe) {
        std::cout << "Error\n";
    }
}


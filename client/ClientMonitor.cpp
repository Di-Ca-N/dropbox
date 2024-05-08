#include <memory>
#include <unistd.h>

#include "ClientMonitor.hpp"
#include "ClientState.hpp"
#include "Connection.hpp"

ClientMonitor::ClientMonitor(std::shared_ptr<ClientState> clientState,
        std::shared_ptr<Connection> connection) {
    this->clientState = clientState;
    this->connection = connection;
}

void ClientMonitor::run() {
    while (*clientState == ClientState::STATE_ACTIVE) {
        // TODO
    }
}


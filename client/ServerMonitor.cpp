#include <memory>
#include <unistd.h>
#include <iostream>

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

    // FIXME: Essa é uma forma MUITO ruim de esperar que o estado seja ativo. 
    // Precisamos disso para garantir que o próximo loop vai entrar, senão o syncRead nunca inicia
    while (clientState->get() != AppState::STATE_ACTIVE);

    while (clientState->get() == AppState::STATE_ACTIVE) {
        operation = connection->syncRead();

        if (operation.has_value())
            history->pushEvent(operation.value());
    }
}


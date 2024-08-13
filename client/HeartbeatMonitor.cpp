#include "HeartbeatMonitor.hpp"

#include <iostream>

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
            try {
                connection->retryConnection();
            } catch (BinderConnectionError) {
                std::cout << "Connection with binder was broken\n";
            } catch (ServerConnectionError) {
                // Não faz nada
            } catch (ErrorReply e) {
                std::cout << "Error: " << e.what() << "\n";
            } catch (UnexpectedMsgType e) {
                std::cout << "Unexpected response 4" << e.what() << "\n";
            } catch (BrokenPipe) {}
        }
    }
}

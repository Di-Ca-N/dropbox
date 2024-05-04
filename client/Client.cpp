#include <memory>
#include <string>
#include <thread>

#include "Client.hpp"
#include "ClientState.hpp"
#include "Connection.hpp"
#include "CLI.hpp"

void Client::run(std::string username, std::string ip, int port) {
    std::shared_ptr<ClientState> clientState =
        std::make_shared<ClientState>(ClientState::STATE_ACTIVE);

    std::shared_ptr<Connection> connection =
        std::make_shared<Connection>(Connection());

    connection->connectToServer(username, ip, port);

    cli = std::make_unique<CLI>(CLI(clientState, connection));

    std::thread cliThread(&CLI::run, std::ref(*cli));

    cliThread.join();
}

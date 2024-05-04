#ifndef CLI_H
#define CLI_H

#include <memory>

#include "ClientState.hpp"
#include "CLICallback.hpp"
#include "Connection.hpp"

class CLI {
    std::shared_ptr<ClientState> clientState;
    std::unique_ptr<CLICallback> callback;

public:
    CLI(std::shared_ptr<ClientState> clientState,
            std::shared_ptr<Connection> connection);
    void run();
};

#endif

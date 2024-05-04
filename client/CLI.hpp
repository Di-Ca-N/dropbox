#ifndef CLI_H
#define CLI_H

#include <memory>

#include "ClientState.hpp"

class CLI {
    std::shared_ptr<ClientState> clientState;

    void getSyncDir();

public:
    CLI();
    void run(std::string username, std::string ip, int port);
};

#endif

// ServerRegistry.hpp

#pragma once

#include <mutex>

#include "Messages.hpp"

class ServerRegistry {
private:
    std::mutex mtx;
    ServerAddress lastServerAddress;

public:
    ServerAddress getLastServerAddress();
    void setLastServerAddress(ServerAddress address);
};

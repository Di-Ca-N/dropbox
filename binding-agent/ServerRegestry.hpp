// ServerRegistry.hpp

#pragma once

#include <mutex>

#include "common/Messages.hpp"

class ServerRegistry {
private:
    std::mutex mtx;
    ServerAddress lastServerAddress;

public:
    ServerAddress getLastServerAddress();
    void setLastServerAddress(ServerAddress address);
};

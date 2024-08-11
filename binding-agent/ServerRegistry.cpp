// ServerResgistry.cpp

#include "ServerRegistry.hpp"

ServerAddress ServerRegistry::getLastServerAddress() {
    const std::lock_guard<std::mutex> lock(mtx);
    return lastServerAddress;
}

void ServerRegistry::setLastServerAddress(ServerAddress address) {
    const std::lock_guard<std::mutex> lock(mtx);
    this->lastServerAddress = address;
}

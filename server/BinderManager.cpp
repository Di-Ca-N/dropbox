#include <iostream>
#include <unistd.h>

#include "handlers/BinderHandler.hpp"
#include "BinderManager.hpp"

BinderManager::BinderManager(ServerAddress binderAddress) {
    this->binderAddress = binderAddress;
}

void BinderManager::notifyBinder(ServerAddress primaryAddress) {
    int binderSock = createSocket(binderAddress.ip, binderAddress.port);

    if (binderSock == -1) {
        std::cout << "Couldn't connect to binder\n";
        return;
    }

    auto handler = BinderHandler(binderSock);
    handler.sendAddress(primaryAddress);

    close(binderSock);
}

int BinderManager::createSocket(in_addr_t ip, in_port_t port) {
    int binderSock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = port;
 
    if (connect(binderSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        return -1;
    }

    return binderSock;
}

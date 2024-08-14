#include <iostream>

#include "BinderHandler.hpp"

BinderHandler::BinderHandler(int binderSock) {
    this->binderSock = binderSock;
}

void BinderHandler::sendAddress(ServerAddress primaryAddress) {
    try {
        sendServerAddress(binderSock, primaryAddress);
        waitConfirmation(binderSock);
    } catch (BrokenPipe) {
        std::cout << "Connection with binder was broken\n";
    } catch (UnexpectedMsgType &e) {
        std::cout << e.what() << "\n";
    }
}

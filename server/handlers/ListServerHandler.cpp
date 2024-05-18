#include "ListServerHandler.hpp"

ListServerHandler::ListServerHandler(std::string username, int clientSocket) {
    this->username = username;
    this->clientSocket = clientSocket;
}

void ListServerHandler::run() {
}


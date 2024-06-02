#include "ListServerHandler.hpp"

ListServerHandler::ListServerHandler(std::string username, ServerSocket clientSocket) {
    this->username = username;
    this->clientSocket = clientSocket;
}

void ListServerHandler::run() {
}


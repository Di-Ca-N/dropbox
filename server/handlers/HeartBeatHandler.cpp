#include "HeartBeatHandler.hpp"
#include <thread>

#include "Messages.hpp"

HeartBeatHandler::HeartBeatHandler(int remoteSocket) {
    this->remoteSocket = remoteSocket;
}

void HeartBeatHandler::run() {
    sendOk(remoteSocket);
    while (true) {
        sendHeartbeat(remoteSocket);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
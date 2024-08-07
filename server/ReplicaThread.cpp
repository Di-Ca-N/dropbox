#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ReplicaThread.hpp"

void ReplicaThread::getServerUpdates(int socketDescr) {
   while (true) {
        UpdateType updateType = receiveUpdateType(socketDescr);
        
        switch (updateType) {
            case UpdateType::UPDATE_CONNECTION:
                sendOk(socketDescr);
                break;
            default:
                break;
        }

    }
    close(socketDescr);
}

void ReplicaThread::run(int &socketDescr, ReplicaManager* replicaManager) {
    // Start the thread to get server updates
    replicaThread = std::thread(&ReplicaThread::getServerUpdates, this, socketDescr);
}


ReplicaThread::~ReplicaThread() {
    if (replicaThread.joinable()) {
        replicaThread.join(); // Wait for the thread to finish before destroying the object
    }
}
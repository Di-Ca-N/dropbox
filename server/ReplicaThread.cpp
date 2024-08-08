#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ReplicaThread.hpp"

void ReplicaThread::getServerUpdates(int socketDescr, ReplicaManager* replicaManager) {
   while (true) {
        UpdateType updateType = receiveUpdateType(socketDescr);
        
        switch (updateType) {
            case UpdateType::UPDATE_CONNECTION:
                getNewReplica(socketDescr, replicaManager);
                break;
            case UpdateType::UPDATE_FILE_OP:
                break;
            case UpdateType::UPDATE_CONNECTION_START:
                sendOk(socketDescr);
                break;
            default:
                break;
        }

    }
    close(socketDescr);
}

void ReplicaThread::getNewReplica(int socketDescr, ReplicaManager* replicaManager) {
    sendOk(socketDescr);
    replicaData = receiveReplicaData(socketDescr);
    replicaManager->pushReplica(replicaData.replicaId, replicaData.replicaIp, replicaData.socketDescr);
    std::cout << "getNewReplica" << std::endl;
    replicaManager->printReplicas();

}

void ReplicaThread::run(int &socketDescr, ReplicaManager* replicaManager) {
    replicaThread = std::thread(&ReplicaThread::getServerUpdates, this, socketDescr, replicaManager);
}


ReplicaThread::~ReplicaThread() {
    if (replicaThread.joinable()) {
        replicaThread.join(); // Wait for the thread to finish before destroying the object
    }
}
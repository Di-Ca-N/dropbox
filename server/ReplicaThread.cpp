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
            case UpdateType::UPDATE_CONNECTION_EQUAL:
                sendOk(socketDescr);
                break;
            case UpdateType::UPDATE_CONNECTION_END:
                removeReplica(socketDescr, replicaManager);
                break;
            default:
                break;
        }

    }
    close(socketDescr);
}

void ReplicaThread::getNewReplica(int socketDescr, ReplicaManager* replicaManager) {
    try {
        replicaData = receiveReplicaData(socketDescr);
        replicaManager->pushReplica(replicaData.replicaId, replicaData.replicaIp, replicaData.socketDescr);
        std::cout << "getNewReplica" << std::endl;
        replicaManager->printReplicas();
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }

}

void ReplicaThread::removeReplica(int socketDescr, ReplicaManager* replicaManager) {
    try {
        replicaData = receiveReplicaData(socketDescr);
        replicaManager->popReplica(replicaData.replicaId);
        std::cout << replicaData.replicaId << std::endl;
        std::cout << "removeReplica" << std::endl;
        replicaManager->printReplicas();
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }

}

void ReplicaThread::run(int &socketDescr, ReplicaManager* replicaManager) {
    replicaThread = std::thread(&ReplicaThread::getServerUpdates, this, socketDescr, replicaManager);
}


ReplicaThread::~ReplicaThread() {
    if (replicaThread.joinable()) {
        replicaThread.join(); // Wait for the thread to finish before destroying the object
    }
}
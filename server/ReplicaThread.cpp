#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ReplicaThread.hpp"

void ReplicaThread::getServerUpdates(int socketDescr, ReplicaManager* replicaManager) {
    try {
        while (true) {
            UpdateType updateType = receiveUpdateType(socketDescr);
            
            switch (updateType) {
                case UpdateType::UPDATE_CONNECTION:
                    getNewReplica(socketDescr, replicaManager);
                    break;
                case UpdateType::UPDATE_FILE_OP:
                    break;
                case UpdateType::UPDATE_CONNECTION_END:
                    removeReplica(socketDescr, replicaManager);
                    break;
                default:
                    break;
            }
        }
    } catch (BrokenPipe) {}
    close(socketDescr);
}

void ReplicaThread::getNewReplica(int socketDescr, ReplicaManager* replicaManager) {
    try {
        replicaData = receiveReplicaData(socketDescr);
        replicaManager->pushReplica(replicaData.replicaId, replicaData.replicaAddr, replicaData.socketDescr);
        std::cout << "Updated replica list:" << std::endl;
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
    int replicaId;

    try {
        replicaId = receiveReplicaId(socketDescr);
        replicaManager->popReplica(replicaId);
        std::cout << replicaId << std::endl;
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
    replicaThread.detach();
    // if (replicaThread.joinable()) {
    //     replicaThread.join(); // Wait for the thread to finish before destroying the object
    // }
}
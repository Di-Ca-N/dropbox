#include "ReplicaConnectionHandler.hpp"

ReplicaConnectionHandler::ReplicaConnectionHandler(int socketDescr, int replicaId, ServerAddress replicaAddr, ReplicaManager* replicaManager)
    : replicaId(replicaId), replicaAddr(replicaAddr), replicaSock(socketDescr), replicaManager(replicaManager) {

}

void ReplicaConnectionHandler::run() {
    sendOk(replicaSock);
    UpdateType UpdateType = receiveUpdateType(replicaSock);

    switch(UpdateType) {
        case UpdateType::UPDATE_CONNECTION_START:
            UpdateConnectionStart();
            break;
        case UpdateType::UPDATE_CONNECTION:
            break;
        case UpdateType::UPDATE_FILE_OP:
            break;
        default:
            break;
    }
}

void ReplicaConnectionHandler:: UpdateConnectionStart() {
    sendOk(replicaSock);
    replicaManager->pushReplica(replicaId, replicaAddr, replicaSock);
    replicaManager->sendAllReplicas(replicaSock);
    replicaManager->updateReplica(replicaId, UpdateType::UPDATE_CONNECTION);
    
}
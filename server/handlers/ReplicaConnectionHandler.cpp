#include "ReplicaConnectionHandler.hpp"

ReplicaConnectionHandler::ReplicaConnectionHandler(int socketDescr, int replicaId, ServerAddress replicaAddr, ReplicaManager* replicaManager)
    : replicaId(replicaId), replicaAddr(replicaAddr), replicaSock(socketDescr), replicaManager(replicaManager) {

}

void ReplicaConnectionHandler::run() {
    try {
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
    } catch (BrokenPipe e) {
        replicaManager->popReplica(replicaId);
        replicaManager->removeReplica(replicaId, UpdateType::UPDATE_CONNECTION_END);
        replicaManager->printReplicas();
        throw e;
    }
}

void ReplicaConnectionHandler:: UpdateConnectionStart() {
    sendOk(replicaSock);
    replicaManager->pushReplica(replicaId, replicaAddr, replicaSock);
    replicaManager->sendAllReplicas(replicaSock);
    replicaManager->updateReplica(replicaId, UpdateType::UPDATE_CONNECTION);
    replicaManager->sendAllFiles(replicaSock);
}
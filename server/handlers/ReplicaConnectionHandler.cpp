#include "ReplicaConnectionHandler.hpp"

ReplicaConnectionHandler::ReplicaConnectionHandler(int socketDescr, int replicaId, ServerAddress replicaAddr, ReplicaManager* replicaManager)
    : replicaId(replicaId), replicaAddr(replicaAddr), replicaSock(socketDescr), replicaManager(replicaManager) {

}

void ReplicaConnectionHandler::run() {
    try {
        sendOk(replicaSock);
        replicaManager->pushReplica(replicaId, replicaAddr, replicaSock);
        replicaManager->sendAllReplicas(replicaSock);
        replicaManager->updateReplica(replicaId, UpdateType::UPDATE_CONNECTION);
    } catch (BrokenPipe e) {
        replicaManager->popReplica(replicaId);
        replicaManager->removeReplica(replicaId, UpdateType::UPDATE_CONNECTION_END);
        replicaManager->printReplicas();
        throw e;
    }
}

void ReplicaConnectionHandler:: UpdateConnectionStart() {

    //replicaManager->sendAllFiles(replicaSock);
}
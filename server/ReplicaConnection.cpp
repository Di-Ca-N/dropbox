#include <iostream>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utils.hpp"

#include "ReplicaConnection.hpp"


ReplicaConnection::ReplicaConnection(int replicaId) {
    this->replicaId = replicaId;
}

bool ReplicaConnection::setConnection(ServerAddress primaryAddr, int myPort, ReplicaManager* replicaManager) {
    createSocket(this->replicaSock, primaryAddr);
    if(!replicaAuth(this->replicaSock, replicaId, myPort)) return false;
    if(!createUpdateType(this->replicaSock)) return false;
    initializeReplicaManager(this->replicaSock, replicaManager);
    runReplicaThread(this->replicaSock, replicaManager);

    return true;
}

void ReplicaConnection::createSocket(int &socketDescr, ServerAddress primaryAddr) {
    int serverConnection = openSocketTo(primaryAddr);

    this->replicaSock = serverConnection;
}

bool ReplicaConnection::replicaAuth(int &socketDescr, int replicaId, uint16_t port) {
    AuthData authData = { .type=AuthType::AUTH_REPLICA };
    authData.replicaData.replicaId = replicaId;
    authData.replicaData.replicaAddr.port = port;
    
    try {
        sendAuth(socketDescr, authData);
        AuthData authResponse = receiveAuth(socketDescr);
        this->replicaIpAddress = authResponse.replicaData.replicaAddr.ip;

    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return false;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return false;
    } catch (BrokenPipe) {
        return false;
    }

    return true;
}

bool ReplicaConnection::createUpdateType(int &socketDescr) {

    try {
        sendUpdate(socketDescr);
        waitConfirmation(socketDescr);
        return true;
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return false;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return false;
    } catch (BrokenPipe) {
        return false;
    }
}

void ReplicaConnection::runReplicaThread(int &socketDescr, ReplicaManager* replicaManager) {
    replicaThread = std::make_shared<ReplicaThread>();
    replicaThread->run(socketDescr, replicaManager);
}

void ReplicaConnection::initializeReplicaManager(int &socketDescr, ReplicaManager* replicaManager) {
    ReplicaData replicaData;
    int numReplicas = 0;

    sendUpdateType(socketDescr, UpdateType::UPDATE_CONNECTION_START);
    waitConfirmation(socketDescr);

    numReplicas = receiveNumFiles(socketDescr);
    sendOk(socketDescr);
    
    for(int i = 0; i < numReplicas; i++) {
        replicaData = receiveReplicaData(socketDescr);
        replicaManager->pushReplica(replicaData.replicaId, replicaData.replicaAddr, replicaData.socketDescr);
    }
    std::cout << "Known replicas:\n"; 
    replicaManager->printReplicas();    
}


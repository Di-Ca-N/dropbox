#include <iostream>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "ReplicaConnection.hpp"


ReplicaConnection::ReplicaConnection(int replicaId) {
    this->replicaId = replicaId;
}

bool ReplicaConnection::setConnection(std::string ip, int port, ReplicaManager* replicaManager) {
    createSocket(this->replicaSock, ip, port);
    if(!replicaAuth(this->replicaSock, replicaId)) return false;
    if(!createUpdateType(this->replicaSock)) return false;
    initializeReplicaManager(this->replicaSock, replicaManager);
    runReplicaThread(this->replicaSock, replicaManager);

    return true;
}

void ReplicaConnection::createSocket(int &socketDescr, std::string ip, int port) {
    int serverConnection = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    inet_aton(ip.data(), &addr.sin_addr);
 
    if (connect(serverConnection, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cout << "Error when connecting to server\n";
        return;
    }

    this->replicaSock = serverConnection;
}

bool ReplicaConnection::replicaAuth(int &socketDescr, int replicaId) {
    AuthData authData = { .type=AuthType::AUTH_REPLICA };
    authData.replicaData.replicaId = replicaId;
    
    try {
        sendAuth(socketDescr, authData);
        AuthData authResponse = receiveAuth(socketDescr);
        this->replicaIpAddress = authResponse.replicaData.ipAddress;

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
        replicaManager->pushReplica(replicaData.replicaId, replicaData.replicaIp, replicaData.socketDescr);
    }

    std::cout << "initializeReplicaManager" << std::endl;
    replicaManager->printReplicas();
}


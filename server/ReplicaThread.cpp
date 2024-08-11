#include <iostream>
#include <cstring>
#include <unistd.h>
#include <filesystem>
#include "utils.hpp"
#include "ReplicaThread.hpp"

void ReplicaThread::getServerUpdates(ReplicaManager* replicaManager, int replicaId, uint16_t port, ServerAddress primaryAddr) {
    int socketDescr = openSocketTo(primaryAddr);
    if (socketDescr == -1) return;

    AuthData authData = { .type=AuthType::AUTH_REPLICA };
    authData.replicaData.replicaId = replicaId;
    authData.replicaData.replicaAddr.port = port;

    try {
        sendAuth(socketDescr, authData);
        AuthData authResponse = receiveAuth(socketDescr);
        //this->replicaIpAddress = authResponse.replicaData.replicaAddr.ip;

        sendUpdate(socketDescr);
        waitConfirmation(socketDescr);

        initializeReplicaManager(socketDescr, replicaManager);
        createDir(socketDescr);

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

void ReplicaThread::initializeReplicaManager(int socketDescr, ReplicaManager *replicaManager) {
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

void ReplicaThread::createDir(int socketDescr) {
    int numDir = 0;
    std::string dirName;
    DirData dirData;

    try {
        numDir = receiveNumFiles(socketDescr);
        sendOk(socketDescr);
    
        for(int i = 0; i < numDir; i++) {
            dirData = receiveDirName(socketDescr);
            std::string dirName(dirData.dirName, dirData.dirnameLen);
            std::filesystem::create_directory(dirName + std::to_string(socketDescr));
            getDirFiles(socketDescr, dirName + std::to_string(socketDescr));
        }
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}

void ReplicaThread::getDirFiles(int socketDescr, std::string dirName) {
    int numFile = 0;
    FileId fileId;
    std::filesystem::path baseDir(dirName.c_str());

    try {
        numFile = receiveNumFiles(socketDescr);
        sendOk(socketDescr);

        for(int i = 0; i < numFile; i++) {
            fileId = receiveFileId(socketDescr);
            std::string filename(fileId.filename, fileId.filenameSize);
            std::ofstream file(baseDir / filename, std::fstream::binary);
        
            if (file) {
                sendOk(socketDescr);
            } else {
                sendError(socketDescr, "Could not create file");
            }
        }
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}

void ReplicaThread::run(ReplicaManager* replicaManager, int replicaId, uint16_t port, ServerAddress primaryAddr) {
    replicaThread = std::thread(&ReplicaThread::getServerUpdates, this, replicaManager, replicaId, port, primaryAddr);
    replicaThread.detach();
}

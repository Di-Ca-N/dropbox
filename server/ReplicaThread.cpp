#include <iostream>
#include <cstring>
#include <unistd.h>
#include <filesystem>
#include "utils.hpp"
#include "ReplicaThread.hpp"

void ReplicaThread::getServerUpdates(ReplicaManager* replicaManager, int replicaId, uint16_t port, ServerAddress primaryAddr) {
    int primarySock = openSocketTo(primaryAddr);
    if (primarySock == -1) return;

    AuthData authData = {
        .type=AuthType::AUTH_REPLICA,
        .replicaData={
            .replicaAddr={
                .port=port
            },
            .replicaId=replicaId
        }
    };
    try {
        sendAuth(primarySock, authData);
        receiveAuth(primarySock);

        sendMessage(primarySock, MsgType::MSG_REPLICATION, nullptr, 0);
        waitConfirmation(primarySock);

        int numReplicas = receiveNumFiles(primarySock);
        sendOk(primarySock);

        for (int i = 0; i < numReplicas; i++) {
            ReplicaData replicaData = receiveReplicaData(primarySock);
            replicaManager->pushReplica(replicaData.replicaId, replicaData.replicaAddr, replicaData.socketDescr);
        }
        sendOk(primarySock);
        std::cout << "Known replicas:\n"; 
        replicaManager->printReplicas();

        while (true) {
            UpdateType updateType = receiveUpdateType(primarySock);
            sendOk(primarySock);

            std::cout << "Got update from primary\n";
            switch (updateType) {
                case UpdateType::UPDATE_CONNECTION:
                    getNewReplica(primarySock, replicaManager);
                    break;
                
                case UpdateType::UPDATE_FILE_OP:
                    handleFileOp(primarySock);
                    break;

                default:
                    break;
            }
        }
    } catch (BrokenPipe) {

    }
    close(primarySock);
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
            std::filesystem::create_directory(dirName + "aa");
            getDirFiles(socketDescr, dirName + "aa");
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

            receiveFileData(socketDescr, fileId.totalBlocks, file);
        }
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}

void ReplicaThread::handleFileOp(int socketDescr) {
    sendOk(socketDescr);

    FileOpType opType = receiveFileOperation(socketDescr);
    sendOk(socketDescr);

    switch (opType) {
        case FileOpType::FILE_MODIFY:
            handleModify(socketDescr);
            break;
        
        default:
            break;
    }
    // try {
    //     sendOk(socketDescr);

    //     opType = receiveFileOperation(socketDescr);

    //     switch (opType)
    //     {
    //         case FileOpType::FILE_MODIFY:
    //             handleModify(socketDescr);
    //             break;
    //         case FileOpType::FILE_DELETE:
    //             break;
    //         default:
    //             break;
    //     }
    // } catch (UnexpectedMsgType) {
    //     std::cout << "Unexpected response.\n";
    //     return;
    // } catch (ErrorReply e) {
    //     std::cout << "Error: " << e.what() << "\n";
    //     return;
    // }
}

void ReplicaThread::handleModify(int socketDescr) {
    sendOk(socketDescr);

    DirData dirData = receiveDirName(socketDescr);
    std::string dirName(dirData.dirName, dirData.dirnameLen);
    std::filesystem::path baseDir(dirName.c_str());
    std::filesystem::create_directories(baseDir);

    std::cout << dirName << "\n";
    sendOk(socketDescr);

    FileId fileId = receiveFileId(socketDescr);
    std::string filename(fileId.filename, fileId.filenameSize);
    std::cout << "Got updates on file" << filename << "\n";

    std::ofstream file(baseDir / filename, std::fstream::binary);

    if (file) {
        sendOk(socketDescr);
    } else {
        std::cout << "error\n";
        sendError(socketDescr, "Could not create file");
        return;
    }
    receiveFileData(socketDescr, fileId.totalBlocks, file);
    sendOk(socketDescr);

    
    std::cout << "Got updates on file" << filename << " SUCCESS\n";
    // FileId fileId;
    // DirData dirData;
    // std::string dirName;
    
    // try {
    //     sendOk(socketDescr);

    //     dirData = receiveDirName(socketDescr);
    //     std::string dirName(dirData.dirName, dirData.dirnameLen);
    //     std::cout << dirName << std::endl;
    //     std::filesystem::path baseDir(dirName.c_str());
    //     sendOk(socketDescr);
        
    //     fileId = receiveFileId(socketDescr);
    //     std::string filename(fileId.filename, fileId.filenameSize);
    //     std::ofstream file(baseDir/ filename, std::fstream::binary);
        
    //     if (file) {
    //         sendOk(socketDescr);
    //         std::cout << "aqui" << std::endl;
    //     } else {
    //         sendError(socketDescr, "Could not create file");
    //     }
        
    //     receiveFileData(socketDescr, fileId.totalBlocks, file);
    //     sendOk(socketDescr);
        
    // } catch (UnexpectedMsgType) {
    //     std::cout << "Unexpected response.\n";
    //     return;
    // } catch (ErrorReply e) {
    //     std::cout << "Error: " << e.what() << "\n";
    //     return;
    // }
}


void ReplicaThread::run(ReplicaManager* replicaManager, int replicaId, uint16_t port, ServerAddress primaryAddr) {
    replicaThread = std::thread(&ReplicaThread::getServerUpdates, this, replicaManager, replicaId, port, primaryAddr);
    replicaThread.detach();
    
}

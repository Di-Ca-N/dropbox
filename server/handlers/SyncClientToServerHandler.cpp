#include "SyncClientToServerHandler.hpp"

#include <fstream>
#include <iostream>

SyncClientToServerHandler::SyncClientToServerHandler(
    std::string username, int clientSocket, 
    int deviceId, DeviceManager* deviceManager, ReplicaManager *replicaManager
) {
    this->username = username;
    this->clientSocket = clientSocket;
    this->deviceId = deviceId;
    this->deviceManager = deviceManager;
    this->replicaManager = replicaManager;
    this->baseDir = std::filesystem::path(username.c_str());
}

void SyncClientToServerHandler::run() {
    try {
        sendOk(clientSocket);

        while (true) {
            FileOpType fileOp = receiveFileOperation(clientSocket);
            sendOk(clientSocket);

            switch (fileOp) {
                case FileOpType::FILE_MODIFY:
                    this->handleFileChange();
                    break;
                
                case FileOpType::FILE_DELETE:
                    this->handleFileDelete();
                    break;
                
                case FileOpType::FILE_MOVE:
                    break;
    
                default:
                    break;
            }
        }
    } catch (ErrorReply) {
        
    }
}

void SyncClientToServerHandler::handleFileChange() {
    FileId fileId = receiveFileId(clientSocket);
    sendOk(clientSocket);

    std::string filename(fileId.filename, fileId.filenameSize);
    std::filesystem::path filepath = baseDir / filename;

    std::ofstream file(filepath, std::ios::binary);
    
    if (!file) {
        return;
    }

    receiveFileData(clientSocket, fileId.totalBlocks, file);
   

    file.flush(); // Ensure the file is completely saved in disk before notifying other devices

    FileOperation op;
    filename.copy(op.filename, MAX_FILENAME);
    op.filenameSize = filename.size();
    op.type = FileOpType::FILE_MODIFY;
    
    replicaManager->notifyAllReplicas(op, username);

    sendOk(clientSocket);
    deviceManager->notifyOtherDevices(op, deviceId);
}

void SyncClientToServerHandler::handleFileDelete() {
    FileId fileId = receiveFileId(clientSocket);

    std::string filename(fileId.filename, fileId.filenameSize);
    std::filesystem::path filepath = baseDir / filename;

    //std::cout << "Removing filepath " << filepath << "\n";
    std::filesystem::remove(filepath);

    FileOperation op;
    filename.copy(op.filename, MAX_FILENAME);
    op.filenameSize = filename.size();
    op.type = FileOpType::FILE_DELETE;

    replicaManager->notifyAllReplicas(op, username);

    sendOk(clientSocket);
    deviceManager->notifyOtherDevices(op, deviceId);
}

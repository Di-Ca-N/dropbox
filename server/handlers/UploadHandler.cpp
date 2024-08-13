#include "UploadHandler.hpp"
#include "Messages.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

UploadHandler::UploadHandler(std::string username, int clientSocket, DeviceManager *deviceManager, ReplicaManager *replicaManager) {
    this->clientSocket = clientSocket;
    this->username = username;
    this->deviceManager = deviceManager;
    this->replicaManager = replicaManager;
}

void UploadHandler::run(){
    std::filesystem::path baseDir(username.c_str());

    try {
        sendOk(clientSocket);

        FileId fileId = receiveFileId(clientSocket);
        
        std::string filename(fileId.filename, fileId.filenameSize);

        std::ofstream file(baseDir / filename, std::fstream::binary);

        if (file) {
            sendOk(clientSocket);
        } else {
            sendError(clientSocket, "Could not create file");
        }

        receiveFileData(clientSocket, fileId.totalBlocks, file);
        
        file.flush(); // Ensure the entire file is written to disk before synchronization

        FileOperation op = {
            .type=FileOpType::FILE_MODIFY,
        };

        filename.copy(op.filename, MAX_FILENAME);
        op.filenameSize = fileId.filenameSize;

        replicaManager->notifyAllReplicas(op, username);
        sendOk(clientSocket);
        deviceManager->notifyAllDevices(op);
    } catch (UnexpectedMsgType) {
        sendError(clientSocket, "Unexpected message");
    }
}


        

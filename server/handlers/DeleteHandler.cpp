#include "DeleteHandler.hpp"

#include <filesystem>
#include <iostream>

#include "Messages.hpp"

DeleteHandler::DeleteHandler(std::string username, int clientSocket, DeviceManager *deviceManager, ReplicaManager *replicaManager) {
    this->clientSocket = clientSocket;
    this->username = username;
    this->deviceManager = deviceManager;
    this->replicaManager = replicaManager;
}

void DeleteHandler::run() {
   std::filesystem::path baseDir(username.c_str());
    try {
        sendOk(clientSocket);
        FileId fid = receiveFileId(clientSocket);
        std::string filename(fid.filename, fid.filename + fid.filenameSize);
        std::filesystem::path filepath = baseDir / filename;

        if (std::filesystem::remove(filepath)) {
            FileOperation fileOp;
            filename.copy(fileOp.filename, MAX_FILENAME);
            fileOp.filenameSize = filename.size();
            fileOp.type = FileOpType::FILE_DELETE;

            replicaManager->notifyAllReplicas(fileOp, username);
    
            sendOk(clientSocket);
            deviceManager->notifyAllDevices(fileOp);
        } else {
            sendError(clientSocket, "File does not exist");
        }
    } catch (UnexpectedMsgType) {
        sendError(clientSocket, "Unexpected Messsage Type\n");
    }
}


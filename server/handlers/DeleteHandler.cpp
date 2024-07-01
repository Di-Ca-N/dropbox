#include "DeleteHandler.hpp"

#include <filesystem>

#include "Messages.hpp"

DeleteHandler::DeleteHandler(std::string username, int clientSocket, DeviceManager *deviceManager) {
    this->clientSocket = clientSocket;
    this->username = username;
    this->deviceManager = deviceManager;
}

void DeleteHandler::run() {
   std::filesystem::path baseDir(username.c_str());
    try {
        sendOk(clientSocket);
        FileId fid = receiveFileId(clientSocket);

        std::string filename(fid.filename, fid.filename + fid.fileSize);
        std::filesystem::path filepath = baseDir / filename;

        if (std::filesystem::remove(filepath)) {
            sendOk(clientSocket);

            FileOperation fileOp;
            filename.copy(fileOp.filename, MAX_FILENAME);
            fileOp.filenameSize = filename.size();
            fileOp.type = FileOpType::FILE_DELETE;

            deviceManager->notifyAllDevices(fileOp);
        } else {
            sendError(clientSocket, "File does not exist");
        }
    } catch (UnexpectedMsgType) {
        sendError(clientSocket, "Unexpected Messsage Type\n");
    }
}


#include "SyncServerToClientHandler.hpp"
#include "Messages.hpp"
#include "utils.hpp"

SyncServerToClientHandler::SyncServerToClientHandler(std::string username, int clientSocket, DeviceManager *deviceManager) {
    this->clientSocket = clientSocket;
    this->username = username;
    this->deviceManager = deviceManager;
    this->baseDir = std::filesystem::path(username.c_str());
}

void SyncServerToClientHandler::run(){
    try {
        Device device = deviceManager->registerDevice();

        sendOk(clientSocket);

        while (true) {
            FileOperation op = device.queue->get();

            try {
                sendFileOperation(clientSocket, op.type);
                waitConfirmation(clientSocket);
            } catch (ErrorReply reply) {
                break;
            }

            switch (op.type) {
                case FileOpType::FILE_MODIFY:
                    this->handleFileModify(std::string(op.filename, op.filenameSize));
                    break;
                case FileOpType::FILE_DELETE:
                    this->handleFileDelete(std::string(op.filename, op.filenameSize));
                    break;
                case FileOpType::FILE_MOVE:
                    //this->handleFileMove();
                    break;
            }
        }
    } catch (UnexpectedMsgType) {

    }
}

void SyncServerToClientHandler::handleFileModify(std::string filename) {
    std::filesystem::path filepath = baseDir / filename;

    FileId fid;
    if (buildFileIdFromPath(filepath, &fid)) {
        std::cout << "Couldn't read file\n";
        return;
    }
    std::ifstream file(filepath, std::ios::binary);

    try {
        sendFileId(clientSocket, fid);
        waitConfirmation(clientSocket);

        sendFileData(clientSocket, fid.totalBlocks, file);
        waitConfirmation(clientSocket);
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

void SyncServerToClientHandler::handleFileDelete(std::string filename) {
    try {
        FileId fileId;
        filename.copy(fileId.filename, MAX_FILENAME);
        fileId.filenameSize = filename.size();

        sendFileId(clientSocket, fileId);
        waitConfirmation(clientSocket);
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

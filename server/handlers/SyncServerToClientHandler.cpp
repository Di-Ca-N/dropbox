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
    std::cout << "SyncServerToClient started\n";
    try {
        Device device = deviceManager->registerDevice();

        sendOk(clientSocket);
        
        while (true) {
            std::cout << "Waiting for file op\n";
            FileOperation op = device.queue->get();
            std::cout << "Got file op\n";
            sendFileOperation(clientSocket, op.type);

            try {
                waitConfirmation(clientSocket);
            } catch (ErrorReply reply) {
                break;
            }

            switch (op.type) {
                case FileOpType::FILE_MODIFY:
                    this->handleFileModify(std::string(op.filename, op.filename+op.filenameSize));
                    break;
                case FileOpType::FILE_DELETE:
                    //this->handleFileDelete();
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

#include "SyncServerToClientHandler.hpp"
#include "Messages.hpp"
#include "utils.hpp"

SyncServerToClientHandler::SyncServerToClientHandler(std::string username, int clientSocket, Device &device) {
    this->clientSocket = clientSocket;
    this->username = username;
    this->device = device;
    this->baseDir = std::filesystem::path(username.c_str());
}

void SyncServerToClientHandler::run(){
    try {
        sendOk(clientSocket);

        while (true) {
            FileOperation op = device.queue->get();

            try {
                sendFileOperation(clientSocket, op.type);
                waitConfirmation(clientSocket);
            } catch (ErrorReply reply) {
                break;
            }

            std::string filename(op.filename, op.filenameSize);

            switch (op.type) {
                case FileOpType::FILE_MODIFY:
                    this->handleFileModify(filename);
                    break;
                case FileOpType::FILE_DELETE:
                    this->handleFileDelete(filename);
                    break;
                default:
                    break;
            }
        }
    } catch (UnexpectedMsgType e) {
        std::cout << e.what() << "\n";
    }
}

void SyncServerToClientHandler::handleFileModify(std::string filename) {
    std::filesystem::path filepath = baseDir / filename;

    FileId fid = getFileId(filepath);
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

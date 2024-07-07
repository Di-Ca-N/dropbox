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
    sendOk(clientSocket);

    while (true) {
        FileOperation op = device.queue->get();

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
}

void SyncServerToClientHandler::handleFileModify(std::string filename) {
    std::filesystem::path filepath = baseDir / filename;

    FileId fid;
    if (buildFileIdFromPath(filepath, &fid)) {
        return;
    }
    std::ifstream file(filepath, std::ios::binary);

    try {
        sendFileOperation(clientSocket, FileOpType::FILE_MODIFY);
        waitConfirmation(clientSocket);

        sendFileId(clientSocket, fid);
        waitConfirmation(clientSocket);

        sendFileData(clientSocket, fid.totalBlocks, file);
        waitConfirmation(clientSocket);
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType e) {
        std::cout << e.what() << "\n";
    }
}

void SyncServerToClientHandler::handleFileDelete(std::string filename) {
    try {
        sendFileOperation(clientSocket, FileOpType::FILE_DELETE);
        waitConfirmation(clientSocket);
        
        FileId fileId;
        filename.copy(fileId.filename, MAX_FILENAME);
        fileId.filenameSize = filename.size();

        sendFileId(clientSocket, fileId);
        waitConfirmation(clientSocket);
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType e) {
        std::cout << e.what() << "\n";
    }
}

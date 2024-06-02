#include "UploadHandler.hpp"
#include "Messages.hpp"
#include <fstream>
#include <filesystem>

UploadHandler::UploadHandler(std::string username, ServerSocket clientSocket) {
    this->clientSocket = clientSocket;
    this->username = username;
}

void UploadHandler::run(){
    std::filesystem::path baseDir(username.c_str());

    try {
        clientSocket.sendOk();
        
        FileId fileId = clientSocket.receiveFileId();
        
        std::string filename(fileId.filename, fileId.filename+fileId.filenameSize);

        std::ofstream file(baseDir / filename);

        if (file) {
            clientSocket.sendOk();
        } else {
            clientSocket.sendError("Could not create file");
        }

        clientSocket.receiveFileData(fileId.totalBlocks, file);

        clientSocket.sendOk();
    } catch (UnexpectedMsgTypeException) {
        clientSocket.sendError("Unexpected message");
    }
}


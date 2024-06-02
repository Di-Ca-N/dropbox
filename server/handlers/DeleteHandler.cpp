#include <filesystem>

#include "DeleteHandler.hpp"
#include "Messages.hpp"

DeleteHandler::DeleteHandler(std::string username, ServerSocket clientSocket) {
    this->clientSocket = clientSocket;
    this->username = username;
}

void DeleteHandler::run() {
   std::filesystem::path baseDir(username.c_str());
    try {
        clientSocket.sendOk();
        FileId fid = clientSocket.receiveFileId();

        std::string filename(fid.filename, fid.filename + fid.fileSize);
        std::filesystem::path filepath = baseDir / filename;

        if (std::filesystem::remove(filepath)) {
            clientSocket.sendOk();
        } else {
            clientSocket.sendError("File does not exist");
        }
    } catch (UnexpectedMsgTypeException) {
        clientSocket.sendError("Unexpected Messsage Type\n");
    }
}


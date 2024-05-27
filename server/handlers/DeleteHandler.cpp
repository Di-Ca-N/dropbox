#include "DeleteHandler.hpp"

#include <filesystem>

#include "Messages.hpp"

DeleteHandler::DeleteHandler(std::string username, int clientSocket) {
    this->clientSocket = clientSocket;
    this->username = username;
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
        } else {
            sendError(clientSocket, "File does not exist");
        }
    } catch (UnexpectedMsgType) {
        sendError(clientSocket, "Unexpected Messsage Type\n");
    }
}


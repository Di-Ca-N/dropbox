#include "DownloadHandler.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Messages.hpp"
#include "utils.hpp"


DownloadHandler::DownloadHandler(std::string username, int clientSocket) {
    this->username = username;
    this->clientSocket = clientSocket;
}

void DownloadHandler::run() {
    std::filesystem::path baseDir(username.c_str());

    try {
        sendOk(clientSocket);
        FileId fid = receiveFileId(clientSocket);
        std::string filename(fid.filename, fid.filenameSize);
        std::filesystem::path filepath = baseDir / filename;

        std::ifstream file(filepath, std::ios::binary);

        if (file) {
            sendOk(clientSocket);
        } else {
            sendError(clientSocket, "File not found");
        }

        fid = getFileId(filepath);

        sendFileId(clientSocket, fid);
        waitConfirmation(clientSocket);

        sendFileData(clientSocket, fid.totalBlocks, file);
        waitConfirmation(clientSocket);
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected message\n";
    } catch (ErrorReply e) {
        std::cout << "Error during file download: " << e.what() << "\n";
    }
}

#include "DownloadHandler.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Messages.hpp"

DownloadHandler::DownloadHandler(std::string username, ServerSocket clientSocket) {
    this->username = username;
    this->clientSocket = clientSocket;
}

void DownloadHandler::run() {
    std::filesystem::path baseDir(username.c_str());

    try {
        clientSocket.sendOk();
        FileId fid = clientSocket.receiveFileId();
        std::string filename(fid.filename, fid.filename + fid.filenameSize);
        std::filesystem::path filepath = baseDir / filename;

        std::ifstream file(filepath, std::ios::binary);

        if (file) {
            clientSocket.sendOk();
        } else {
            clientSocket.sendError("File not found");
        }

        file.seekg(file.end);
        u_int64_t fileSize = file.tellg();
        file.seekg(file.beg);
        
        fid.fileSize = fileSize;
        fid.totalBlocks = getNumBlocks(fileSize, MAX_PAYLOAD);
        
        clientSocket.sendFileId(fid);
        clientSocket.waitConfirmation();

        clientSocket.sendFileData(fid.totalBlocks, file);
        clientSocket.waitConfirmation();
    } catch (UnexpectedMsgTypeException) {
        std::cout << "Unexpected message\n";
    } catch (ErrorReply &e) {
        std::cout << "Error during file download: " << e.what() << "\n";
    }
}

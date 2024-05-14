#include "UploadHandler.hpp"
#include "Messages.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

UploadHandler::UploadHandler(std::string username, int clientSocket) {
    this->clientSocket = clientSocket;
    this->username = username;
}

void UploadHandler::run(){
    std::filesystem::path baseDir(username.c_str());

    try {
        sendOk(clientSocket);
        
        FileId fileId = receiveFileId(clientSocket);
        
        std::string filename(fileId.filename, fileId.filename+fileId.filenameSize);

        std::ofstream file(baseDir / filename);

        if (file) {
            sendOk(clientSocket);
        } else {
            sendError(clientSocket, "Could not create file");
        }

        receiveFileData(clientSocket, fileId.totalBlocks, file);

        sendOk(clientSocket);
    } catch (BrokenPipe) {
        std::cout << "Client disconnected during upload\n";
    } catch (UnexpectedMsgType) {
        sendError(clientSocket, "Unexpected message");
    }
}


        

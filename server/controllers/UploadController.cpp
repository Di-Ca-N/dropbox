#include "UploadController.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

#include "Messages.h"
#include "utils.hpp"

UploadController::UploadController(int sock_fd, std::string username) {
    this->sock_fd = sock_fd;
    this->username = username;
}

void UploadController::run() {
    std::filesystem::path baseDir(username.c_str());

    try {
        sendOk(sock_fd);
        FileId fileId = receiveFileId(sock_fd);
        std::cout << "1\n";

        std::ofstream file(baseDir/fileId.filename, std::ios::binary);
        std::cout << "2\n";
        if (file) {
            sendOk(sock_fd);
        } else {
            sendError(sock_fd, "Couldn't create file");
        }
        std::cout << "3\n";
        receiveFileData(sock_fd, fileId.totalBlocks, file);
        std::cout << "4\n";
        sendOk(sock_fd);
        std::cout << "5\n";
    } catch (UnexpectedMsgType) {
        sendError(sock_fd, "Unexpected Msg Type");
        std::cout << "Client sent an unexpected message type";
    }
}

#include "DownloadController.hpp"
#include "Messages.h"

DownloadController::DownloadController(int sock_fd, std::string username) {
    this->sock_fd = sock_fd;
    this->username = username;
}

void DownloadController::run() {
    std::filesystem::path baseDir(username.c_str());

    try {
        sendOk(sock_fd);
        FileId fileId = receiveFileId(sock_fd);
        std::string filename(fileId.filename, fileId.filenameSize);

        std::filesystem::path file = baseDir / filename;

        if (std::filesystem::exists(file)) {
            sendOk(sock_fd);
        } else {
            sendError(sock_fd, "File does not exist on server\n");
            return;
        }
    } catch (UnexpectedMsgType) {
        sendError(sock_fd, "Unknown msg type");
    }
}

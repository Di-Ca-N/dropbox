#include <vector>

#include <sys/stat.h>
#include <string.h>

#include "Messages.hpp"
#include "ListServerHandler.hpp"

ListServerHandler::ListServerHandler(std::string username, int clientSocket) {
    this->username = username;
    this->clientSocket = clientSocket;
}

void ListServerHandler::run() {
    struct stat fileStat;

    FileMeta fileMeta;
    std::vector<FileMeta> fileMetas;
    std::filesystem::path baseDir(username.c_str());

    try {
        sendOk(clientSocket);

        for (const auto& file : std::filesystem::directory_iterator(baseDir)) {
            if (stat(file.path().c_str(), &fileStat) == 0) {
                strncpy(fileMeta.filename, file.path().c_str(), MAX_FILENAME); 
                fileMeta.mTime = fileStat.st_mtime;
                fileMeta.aTime = fileStat.st_atime;
                fileMeta.cTime = fileStat.st_ctime;
                fileMetas.push_back(fileMeta);
            }
        }

        sendNumFiles(clientSocket, fileMetas.size());
        waitConfirmation(clientSocket);

        for (const auto& meta : fileMetas)
            sendFileMeta(clientSocket, meta);

        waitConfirmation(clientSocket);
    } catch (UnexpectedMsgType) {
        sendError(clientSocket, "Unexpected message");
    }
}


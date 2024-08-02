#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

#include "Messages.hpp"

class Connection {
private:
    int deviceId = -1;
    int commandSock = -1;
    int readSock = -1;
    int writeSock = -1;

    void createSocket(int &socketDescr, std::string ip, int port);
    void authenticate(int &socketDescr, std::string username);
    void setWriteConnection(int &socketDescr);
    void sendChange(std::filesystem::path target);
    void sendDelete(std::filesystem::path target);

    void setReadConnection();
    std::optional<FileOperation> syncProcessRead();
    void syncReadChange(FileId &fileId);
    void syncReadDelete(FileId &fileId);
    FileOperation makeFileOperation(FileId &fileId, FileOpType &fileOpType);

public:
    void connectToServer(std::string username, std::string ip, int port);
    void upload(std::filesystem::path filepath);
    void download(std::filesystem::path filepath);
    void delete_(std::filesystem::path filepath);
    std::vector<FileMeta> listServer();
    std::optional<FileOperation> syncRead();
    void syncWrite(FileOpType op, std::filesystem::path target);
};

#endif

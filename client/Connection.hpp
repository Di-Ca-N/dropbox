#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>
#include <filesystem>

#include "Messages.hpp"

class Connection {
private:
    int commandSock = -1;
    int readSock = -1;

    void createSocket(int &socketDescr, std::string ip, int port);
    void authenticate(int &socketDescr, std::string username);
    void setReadConnection(int &socketDescr);

public:
    void connectToServer(std::string username, std::string ip, int port);
    void upload(std::filesystem::path filepath);
    void download(std::filesystem::path filepath);
    void delete_(std::filesystem::path filepath);
    std::vector<FileMeta> listServer();
    void syncRead();
    void syncWrite(FileOpType op, std::filesystem::path target);
};

#endif

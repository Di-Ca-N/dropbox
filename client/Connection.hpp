#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <filesystem>

#include "ClientSocket.hpp"
#include "FileOp.hpp"

class Connection {
private:
    ClientSocket serverSock;

public:
    void connectToServer(std::string username, std::string ip, int port);
    void upload(std::filesystem::path filepath);
    void download(std::filesystem::path filepath);
    void delete_(std::filesystem::path filepath);
    void listServer();
    void syncRead();
    void syncWrite(FileOp op, std::string ogFilename, std::string newFilename);
};

#endif

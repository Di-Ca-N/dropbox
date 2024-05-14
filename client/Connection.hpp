#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>
#include <filesystem>

#include "FileMetadata.hpp"

class Connection {
private:
    int serverSock = -1;

public:
    void connectToServer(std::string username, std::string ip, int port);
    void upload(std::filesystem::path filepath);
    void download(std::filesystem::path filepath);
    void delete_(std::filesystem::path filepath);
    std::vector<FileMetadata> listServer();
};

#endif

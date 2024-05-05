#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>

#include "FileMetadata.hpp"

class Connection {
public:
    void connectToServer(std::string username, std::string ip, int port);
    void upload(std::string filepath);
    void download(std::string filepath);
    void delete_(std::string filepath);
    std::vector<FileMetadata> listServer();
};

#endif

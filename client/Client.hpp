#ifndef CLIENT_H
#define CLIENT_H

#include <memory>

#include "CLI.hpp"
#include "ServerMessageReader.hpp"
#include "FileMonitor.hpp"

class Client {
    std::unique_ptr<CLI> cli;
    std::unique_ptr<ServerMessageReader> serverReader;
    std::unique_ptr<FileMonitor> fileMonitor;

public:
    void run(std::string username, std::string ip, int port);
};

#endif

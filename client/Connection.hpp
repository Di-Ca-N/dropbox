#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <stdexcept>

#include "Messages.hpp"

class Connection {
private:
    int deviceId = -1;
    int binderSock = -1;
    int heartbeatSock = -1;
    int commandSock = -1;
    int readSock = -1;
    int writeSock = -1;

    std::string username;

    in_addr_t convertIpStringToNetwork(std::string ipString);
    in_port_t convertPortIntToNetwork(int port);

    void connectToBinder(in_addr_t &ip, in_port_t &port);
    void connectToServer(in_addr_t &ip, in_port_t &port);

    int createSocket(in_addr_t &ip, in_port_t &port);
    void authenticate(int &socketDescr, std::string username);
    void setWriteConnection(int &socketDescr);
    void sendChange(std::filesystem::path target);
    void sendDelete(std::filesystem::path target);

    void setReadConnection(int socket);
    std::optional<FileOperation> syncProcessRead();
    void syncReadChange(FileId &fileId);
    void syncReadDelete(FileId &fileId);
    FileOperation makeFileOperation(FileId &fileId, FileOpType &fileOpType);

    void setHeartbeatConnection(int socket);

public:
    void connectToService(std::string username, std::string ip, int port);
    void retryConnection();
    bool hearsHeartbeat(int timeout);
    void upload(std::filesystem::path filepath);
    void download(std::filesystem::path filepath);
    void delete_(std::filesystem::path filepath);
    std::vector<FileMeta> listServer();
    std::optional<FileOperation> syncRead();
    void syncWrite(FileOpType op, std::filesystem::path target);
};

class BinderConnectionError : public std::exception {
public:
    BinderConnectionError(){};
};

class ServerConnectionError : public std::exception {
public:
    ServerConnectionError(){};
};

#endif

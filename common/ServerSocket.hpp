// This Socket wrapper ensures consistency in resource acquisition and release

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "BaseSocket.hpp"

class ServerSocket : public BaseSocket {
    struct sockaddr_in clientAddress;

    ServerSocket(int sockfd);
    void setBind(bool value);
    void setListen(bool value);
    bool isBound();
    bool isListening();

public:
    ServerSocket() : BaseSocket() {};
    void bind(int port);
    void listen();
    ServerSocket accept();
    struct sockaddr_in getClientAddress();
};

class SocketBindError : public std::runtime_error {
public:
    SocketBindError(const std::string &message)
        : std::runtime_error(message) {}
};

class SocketListenError : public std::runtime_error {
public:
    SocketListenError(const std::string &message)
        : std::runtime_error(message) {}
};

class SocketAcceptError : public std::runtime_error {
public:
    SocketAcceptError(const std::string &message)
        : std::runtime_error(message) {}
};

#endif

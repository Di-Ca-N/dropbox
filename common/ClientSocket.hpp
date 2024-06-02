// This Socket wrapper ensures consistency in resource acquisition and release

#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "BaseSocket.hpp"

class ClientSocket : public BaseSocket {
public:
    void connect(std::string hostName, int port);
};

class InvalidHostNameException : public std::runtime_error {
public:
    InvalidHostNameException(const std::string &message)
        : std::runtime_error(message) {}
};

class SocketConnectError : public std::runtime_error {
public:
    SocketConnectError(const std::string &message)
        : std::runtime_error(message) {}
};

#endif

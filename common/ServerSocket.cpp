#include <cstring>
#include <sys/socket.h>

#include "ServerSocket.hpp"

ServerSocket::ServerSocket(int sockfd) {
    this->sockfd = sockfd;
    status.set(true);
}

void ServerSocket::bind(int port) {
    struct sockaddr_in serverAddress;

    if (isBound())
        throw SocketBindError("Socket is already bound");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    explicit_bzero(&(serverAddress.sin_zero), 8);

    if (::bind(sockfd,
                (struct sockaddr *) &serverAddress,
                sizeof(serverAddress)) < 0)
        throw SocketBindError("Couldn't bind server socket");

    setBind(true);
}

void ServerSocket::setBind(bool value) {
    status.set(0, value);
}

void ServerSocket::listen() {
    if (!isBound())
        throw SocketListenError("Tried to listen to unbound socket");

    if (isListening())
        throw SocketListenError("Socket is already listening");

    ::listen(sockfd, SOMAXCONN); 

    setListen(true);
}

bool ServerSocket::isBound() {
    return status[0];
}

void ServerSocket::setListen(bool value) {
    status.set(1, value);
}

ServerSocket ServerSocket::accept() {
    int newSock;
    socklen_t addressSize;

    if (!isListening())
        throw SocketAcceptError("Socket wasn't put to listen");

    addressSize = sizeof(struct sockaddr_in);
    if ((newSock = ::accept(sockfd,
                    (struct sockaddr *) &clientAddress,
                    &addressSize)) == -1)
        throw SocketAcceptError("Error while accepting connection");

    setSynced(true);

    return ServerSocket(newSock);
}

bool ServerSocket::isListening() {
    return status[1];
}

struct sockaddr_in ServerSocket::getClientAddress() {
    return clientAddress;
}

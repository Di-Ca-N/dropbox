#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "ClientSocket.hpp"

void ClientSocket::connect(
        std::string hostName,
        int port) {
    struct hostent *server;
    struct sockaddr_in serverAddress;

    if (isSynced())
        throw SocketConnectError("The socket is already connected");

    server = gethostbyname(hostName.c_str());
    if (server == nullptr)
        throw InvalidHostNameException("Host name given is not a valid address");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr = *((struct in_addr *) server->h_addr);
    explicit_bzero(&(serverAddress.sin_zero), 8);

    if (::connect(sockfd,
                (struct sockaddr *) &serverAddress,
                sizeof(serverAddress)) < 0)
        throw SocketConnectError("Couldn't connect to the server socket");

    setSynced(true);
}

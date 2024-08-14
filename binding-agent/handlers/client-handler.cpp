// handlers/client-handler.cpp

#include <iostream>

#include "../ServerRegistry.hpp"
#include "Messages.hpp"
#include "client-handler.hpp"

extern ServerRegistry registry;

void respondToServiceDiscovery(int socket);

void handleClientConnection(int socket, sockaddr_in clientSockAddr) {
    try {
        while (true) {
            Message msg = receiveMessage(socket);

            if (msg.type == MsgType::MSG_STATUS_INQUIRY) {
                sendOk(socket);
                respondToServiceDiscovery(socket);
            } else {
                sendError(socket, "Unrecognized protocol");
            }
        }
    } catch (BrokenPipe) {
        std::cerr << "Client has disconnected\n";
    }
}

void respondToServiceDiscovery(int socket) {
    ServerAddress address = registry.getLastServerAddress();

    try {
        sendServerAddress(socket, address);
        waitConfirmation(socket);
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected message\n";
    } catch (ErrorReply e) {
        std::cout << e.what() << "\n";
    }
}

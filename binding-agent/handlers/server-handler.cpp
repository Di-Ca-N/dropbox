#include <iostream>

#include "../ServerRegistry.hpp"
#include "Messages.hpp"
#include "server-handler.hpp"

extern ServerRegistry registry;

void sendServerRegistry(int socket);

void handleServerConnection(int socket) {
    try {
        Message msg = receiveMessage(socket);

        if (msg.type == MsgType::MSG_SERVER_ADDRESS) { 
            sendOk(socket);
            sendServerRegistry(socket);            
        } else {
            sendError(socket, "Unrecognized protocol");
        }
    } catch (BrokenPipe) {
        std::cerr << "Server has disconnected\n";
    }
}

void sendServerRegistry(int socket) {
    try {
        ServerAddress address = receiveServerAddress(socket);
        sendOk(socket);
        registry.setLastServerAddress(address);
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected message\n";
    } catch (ErrorReply e) {
        std::cout << e.what() << "\n";
    }
}

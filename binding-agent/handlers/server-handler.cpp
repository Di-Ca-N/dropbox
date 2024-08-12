// handlers/server-handler.cpp

#include <iostream>

#include "common/Messages.hpp"
#include "server-handler.hpp"

void handleServerConnection(int socket) {
    try {
        Message msg = receiveMessage(socket);

        if (msg.type == MsgType::MSG_SERVER_REGISTRY) {
            sendOk(socket);
            sendServerRegistry();            
        } else {
            sendError(socket, "Unrecognized protocol");
        }
    } catch () {
        
    }
}

void sendServerRegistry() {
    try {
        // mandar ip e port
        waitConfirmation(socket);
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected message\n";
    } catch (ErrorReply e) {
        std::cout << e.what() << "\n";
    }
}

// std::cout << "A server has just connected\n";
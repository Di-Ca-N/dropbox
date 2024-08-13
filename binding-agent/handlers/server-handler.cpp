#include <iostream>

#include "../ServerRegistry.hpp"
#include "Messages.hpp"
#include "server-handler.hpp"

extern ServerRegistry registry;

void handleServerConnection(int socket) {
    ServerAddress address;

    try {
        address = receiveServerAddress(socket);
        sendOk(socket);
        registry.setLastServerAddress(address);
    } catch (BrokenPipe) {
        std::cerr << "Server has disconnected\n";
    } catch (UnexpectedMsgType &e) {
        std::cerr << e.what() << "\n";
    }

    std::cout << "New Primary: " << address << "\n";
    std::cout << "Server has disconnected\n";
}


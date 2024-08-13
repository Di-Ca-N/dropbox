#include <iostream>

#include "../ServerRegistry.hpp"
#include "Messages.hpp"
#include "server-handler.hpp"

extern ServerRegistry registry;

void handleServerConnection(int socket, sockaddr_in serverSockAddr) {
    std::cout << "chegou aqui\n";
    ServerAddress address;

    try {
        address = receiveServerAddress(socket);
        address.ip = serverSockAddr.sin_addr.s_addr;
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


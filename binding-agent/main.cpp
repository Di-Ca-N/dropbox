#include <iostream>
#include <stdexcept>
#include <thread>
#include <unistd.h>

int convertCharArrayToPort(char *array);

void acceptClientConnections(int port);
void acceptServerConnections(int port);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: binding-agent <client-port> <server-port>\n";
        return 1;
    }

    int clientPort, serverPort;
    try {
        clientPort = convertCharArrayToPort(argv[1]);
        serverPort = convertCharArrayToPort(argv[2]);
    } catch (std::invalid_argument) {
        std::cerr << "Received invalid arguments\n";
        return 1;
    }

    std::thread thread_A(acceptClientConnections, clientPort);
    std::thread thread_B(acceptServerConnections, serverPort);

    thread_A.join();
    thread_B.join();

    return 0;
}

int convertCharArrayToPort(char *array) {
    int port = atoi(array);

    if (port == 0)
        throw std::invalid_argument("Char array does not correspond to a valid port");

    return port;
}

void acceptClientConnections(int port) {
    // TODO
    std::cout << "Client connections will be accepted through port " << port << "\n";
}

void acceptServerConnections(int port) {
    // TODO
    std::cout << "Server connections will be accepted through port " << port << "\n";
}

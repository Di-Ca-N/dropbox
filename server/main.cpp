#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>

#include "Messages.hpp"

void handleClient(int clientSocket) {
    char buffer[256];
    while (true) {
        int bytes = recv(clientSocket, buffer, 256, 0);

        if (bytes == 0) {
            break;
        }
        std::cout << buffer << std::endl;
    }

    close(clientSocket);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: server <port>\n");
        return 1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        fprintf(stderr, "Error on creating socket\n");
        return 1;
    }
    int optVal = 1;
    setsockopt(sock_fd, 1, SO_REUSEADDR, &optVal, sizeof(optVal));

    // Binding do socket na porta 8000
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;
    int err;
    err = bind(sock_fd, (sockaddr*) &addr, sizeof(addr));

    if (err == -1) {
        fprintf(stderr, "Could not bind to port %s\n", argv[1]);
        return 1;
    }

    err = listen(sock_fd, 10); // Socket começa a escutar na porta
    if (err == -1) {
        fprintf(stderr, "Error on listen\n");
        return 1;
    }

    std::cout << "Server listening on port " << argv[1] << "\n";

    std::vector<std::thread> openConnections;

    int c = 0;
    while (true) {
        int clientSocket = accept(sock_fd, nullptr, nullptr);
        std::cout << "Cliente conectou\n";
        openConnections.push_back(std::thread(handleClient, clientSocket));
        c++;

        if (c == 3) break; // Just to test server closing
    }

    // Wait for all clients to close their connections
    for (auto &conn: openConnections) {
        conn.join();
    }

    close(sock_fd);

    return 0;
}
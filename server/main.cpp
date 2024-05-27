#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>

#include "Messages.hpp"
#include "handlers/UploadHandler.hpp"
#include "handlers/DownloadHandler.hpp"
#include "handlers/DeleteHandler.hpp"

void handleClient(int clientSocket) {
    try {
        std::string username = receiveAuth(clientSocket);
        std::filesystem::create_directory(username);
        sendOk(clientSocket);

        while (true) {
            Message msg = receiveMessage(clientSocket);

            switch(msg.type) {
                case MsgType::MSG_UPLOAD:
                    UploadHandler(username, clientSocket).run();
                    break;
                case MsgType::MSG_DOWNLOAD:
                    DownloadHandler(username, clientSocket).run();
                    break;
                case MsgType::MSG_DELETE:
                    DeleteHandler(username, clientSocket).run();
                    break;
                case MsgType::MSG_LIST_SERVER:
                    break;
                default:
                    sendError(clientSocket, "Unrecognized command");
                    break;
            }
        }
    } catch (BrokenPipe) {
        std::cout << "Client disconnected\n";
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
    std::filesystem::current_path(std::filesystem::current_path() / "data");
    int c = 0;
    while (true) {
        int clientSocket = accept(sock_fd, nullptr, nullptr);
        std::cout << "Cliente conectou\n";
        openConnections.push_back(std::thread(handleClient, clientSocket));
        c++;
    }

    // Wait for all clients to close their connections
    for (auto &conn: openConnections) {
        conn.join();
    }

    close(sock_fd);

    return 0;
}

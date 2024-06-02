#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>

#include "ServerSocket.hpp"
#include "Messages.hpp"
#include "handlers/UploadHandler.hpp"
#include "handlers/DownloadHandler.hpp"
#include "handlers/DeleteHandler.hpp"

void handleClient(ServerSocket clientSocket) {
    try {
        std::string username = clientSocket.receiveAuth();
        std::filesystem::create_directory(username);
        clientSocket.sendOk();

        while (true) {
            Message msg = clientSocket.receiveMessage();

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
                    clientSocket.sendError("Unrecognized command");
                    break;
            }
        }
    } catch (BrokenPipeException) {
        std::cout << "Client disconnected\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: server <port>\n");
        return 1;
    }

    ServerSocket sock_fd = ServerSocket();

    sock_fd.bind(atoi(argv[1]));
    sock_fd.listen();

    std::vector<std::thread> openConnections;
    std::filesystem::current_path(std::filesystem::current_path() / "data");
    int c = 0;
    while (true) {
        ServerSocket newSocket = sock_fd.accept();
        std::cout << "Cliente conectou\n";
        openConnections.push_back(std::thread(handleClient, newSocket));
        c++;
    }

    // Wait for all clients to close their connections
    for (auto &conn: openConnections) {
        conn.join();
    }

    return 0;
}

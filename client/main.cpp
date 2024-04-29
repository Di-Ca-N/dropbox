#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <thread>

#include "Messages.h"

std::string username;
std::string serverIp;
int port;

int connectToServer(std::string username, std::string serverIp, int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(serverIp.data(), &addr.sin_addr);

    int err = connect(sock_fd, (sockaddr*) &addr, sizeof(addr));

    if (err == -1) {
        fprintf(stderr, "Error on connecting to server\n");
        close(sock_fd);
        return -1;
    }

    if (sendAuthMsg(sock_fd, username) == -1) {
        fprintf(stderr, "Error authenticating to server\n");
        close(sock_fd);
        return -1;
    };

    return sock_fd;
}

void handleUpload() {
    std::filesystem::path filePath;
    std::cin >> filePath;

    if (!std::filesystem::exists(filePath)) {
        std::cout << "File '" << filePath << "' does not exist\n";
        return;
    }

    if (!std::filesystem::is_regular_file(filePath)) {
        std::cout << "Path '" << filePath << "' is not a regular file\n";
        return;
    }

    std::cout << "Uploading " << filePath << "...\n";

    int serverConnection = connectToServer(username, serverIp, port);

    if (sendUploadMsg(serverConnection) != 0) {
        std::cout << "Error uploading file\n";
        return;
    }

    if (sendFile(serverConnection, filePath) == 0) {
        std::cout << "Done!\n";
    }
}

void syncReader(int serverSocket) {
    Message msg;
    while (true) {
        if (readMessage(serverSocket, &msg) == -1) {
            break;
        };
        printMsg(&msg);
    }
}

void syncWriter(int serverSocket) {

}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: ./client <username> <server_ip_address> <port>\n");
        return 1;
    }

    username = std::string(argv[1]);
    serverIp = std::string(argv[2]);
    port = atoi(argv[3]);

    int sync_fd = connectToServer(username, serverIp, port);

    if (sync_fd == -1)
        return 1;

    if (sendSyncMsg(sync_fd) == -1) {
        fprintf(stderr, "Error while getting sync dir\n");
        close(sync_fd);
        return 1;
    }

    std::thread reader(syncReader, sync_fd);
    std::thread writer(syncWriter, sync_fd);

    while (true) {
        std::cout << "> ";
        std::string cmd;
        std::cin >> cmd;
        
        if (cmd == "upload") {
            handleUpload();
        } else if (cmd == "exit") {
            break;
        }
    }

    close(sync_fd);

    return 0;
}
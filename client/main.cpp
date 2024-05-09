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
#include "utils.hpp"

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

    sendAuth(sock_fd, username);
    waitConfirmation(sock_fd);

    return sock_fd;
}

void handleUpload() {
    std::filesystem::path filepath;
    std::cin >> filepath;

    if (!std::filesystem::exists(filepath)) {
        std::cout << "File " << filepath << " does not exist\n";
        return;
    }

    if (!std::filesystem::is_regular_file(filepath)) {
        std::cout << "File " << filepath << " is not a regular file\n";
        return;
    }

    std::ifstream file(filepath, std::ios::binary);

    if (!file) {
        std::cout << "Couldn't open " << filepath << " for upload\n";
        return;
    }

    file.seekg(std::ios::end);
    u_int64_t fileSize = file.tellg();
    file.seekg(std::ios::beg);
    u_int64_t numBlocks = getNumBlocks(fileSize, MAX_PAYLOAD);
    std::string filename = filepath.filename().string();

    FileId fileId = {
        .totalBlocks=numBlocks,
        .fileSize=fileSize,
        .filenameSize=filename.length(),
    };
    filename.copy(fileId.filename, MAX_FILENAME);
    int serverConnection;
    try {
        serverConnection = connectToServer(username, serverIp, port);
        sendMessage(serverConnection, MsgType::MSG_UPLOAD, nullptr, 0);
        waitConfirmation(serverConnection);
        sendFileId(serverConnection, fileId);
        waitConfirmation(serverConnection);
        sendFileData(serverConnection, numBlocks, file);
        waitConfirmation(serverConnection);

        std::cout << "Upload successful\n";
    } catch (BrokenPipe) {
        std::cout << "Server finished connection during upload\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what();
    }
    close(serverConnection);
}

void handleDownload() {
    std::filesystem::path filepath;
    std::cin >> filepath;
    std::string filename = filepath.filename().string();

    FileId fileId = {
        .filenameSize=filename.length(),
    };
    filename.copy(fileId.filename, MAX_FILENAME);
    int serverConnection;
    try {
        serverConnection = connectToServer(username, serverIp, port);
        sendMessage(serverConnection, MsgType::MSG_DOWNLOAD, nullptr, 0);
        waitConfirmation(serverConnection);

        sendFileId(serverConnection, fileId);
        waitConfirmation(serverConnection);

        std::cout << "Download successful\n";
    } catch (BrokenPipe) {
        std::cout << "Server finished connection during upload\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what();
    }
    close(serverConnection);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: ./client <username> <server_ip_address> <port>\n");
        return 1;
    }

    username = std::string(argv[1]);
    serverIp = std::string(argv[2]);
    port = atoi(argv[3]);

    int sync_fd = connectToServer(username, serverIp, port);

    if (sync_fd == -1)
        return 1;

    while (true) {
        std::cout << "> ";
        std::string cmd;
        std::cin >> cmd;
        
        if (cmd == "upload") {
            handleUpload();
        } else if (cmd == "download") {
            handleDownload();
        } else if (cmd == "exit") {
            break;
        }
    }

    close(sync_fd);

    return 0;
}
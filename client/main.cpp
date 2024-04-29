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

    Message msg = {
        .type=MSG_UPLOAD,
        .len=0
    };

    send(serverConnection, &msg, sizeof(msg), 0);
    readMessage(serverConnection, &msg); // ToDo: solve error

    std::ifstream fileToUpload(filePath, std::ifstream::binary);
    fileToUpload.seekg(0, fileToUpload.end);
    std::streampos fileSize = fileToUpload.tellg();
    fileToUpload.seekg(0, fileToUpload.beg);

    std::cout << "File Size: " << fileSize << " bytes\n";

    unsigned int numBlocks = fileSize / MAX_PAYLOAD;
    if (fileSize % MAX_PAYLOAD > 0) {
        numBlocks++;
    }

    std::string filename = filePath.filename().string();

    FileId payload = {
        .totalBlocks=numBlocks,
        .fileSize=static_cast<u_int64_t>(fileSize),
        .filenameSize=static_cast<u_int8_t>(filename.size()+1),
    };
    filename.copy(payload.filename, MAX_FILENAME);

    msg = {
        .type=MSG_FILE_ID,
        .len=sizeof(payload)
    };
    memcpy(msg.payload, &payload, sizeof(payload));

    send(serverConnection, &msg, sizeof(msg), 0);

    Message filePart = {.type=MSG_FILEPART};
    for (int i = 0; i < numBlocks; i++) {
        fileToUpload.read(filePart.payload, MAX_PAYLOAD);
        filePart.len = fileToUpload.gcount();
        //printMsg(&filePart);
        int sent = send(serverConnection, &filePart, sizeof(filePart), 0);
        //std::cout << "Sent " << sent << " bytes\n";
        if (sent == -1) {
            std::cout << "ERROR on block " << i << "\n";
            break;
        }
    }
    fileToUpload.close();

    Message reply;
    readMessage(serverConnection, &reply);
    //printMsg(&reply);

    if (reply.type == MSG_OK) {
        std::cout << "Done!\n";
    } else {
        std::cout << std::string(reply.payload, reply.payload+reply.len) << "\n";
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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string.h>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <map>
#include <queue>
#include <semaphore.h>

#include "Messages.h"
#include "DeviceManager.h"

std::map<std::string, DeviceManager*> deviceManagers;

void clientSyncReader(int clientSocket, std::string username, Device device) {

}

void clientSyncWriter(int clientSocket, std::string username, Device device) {
    std::cout << "Writer thread\n";
    std::filesystem::path baseDir(username.c_str());

    while (true) {
        FileOperation op = device.queue->get();

        std::cout << "Sending updates on file " << op.file << " to device " << device.id << "\n";

        if (sendFile(clientSocket, baseDir / op.file) == -1) {
            break;
        }
    }
}


void handleSync(int clientSocket, std::string username) {
    // Create Sync dir if not exists
    std::filesystem::create_directory(username);

    deviceManagers.emplace(username, new DeviceManager(username));
    DeviceManager *manager = deviceManagers[username];

    // Register device
    Device device = manager->registerDevice();

    // Start sync threads
    std::cout << "Creating sync threads for user " << username << " device " << device.id << "\n";
    std::thread reader(clientSyncReader, clientSocket, username, device);
    std::thread writer(clientSyncWriter, clientSocket, username, device);

    sendOk(clientSocket);

    reader.join();
    writer.join();

    std::cout << "Device " << device.id << " of " << username << " disconnected\n";
    deviceManagers[username]->disconnectDevice(device.id);
}


void clientUpload(int clientSocket, std::string username) {
    if (sendOk(clientSocket) == -1)
        return;

    FileId fileId;
    if (receiveFileId(clientSocket, &fileId) == -1)
        return;

    std::filesystem::path filename(fileId.filename);
    if (receiveFile(clientSocket, username.c_str() / filename, fileId.totalBlocks) == -1)
        return;

    if (deviceManagers.find(username) != deviceManagers.end()) {
        // Notify all clients about the new file
        deviceManagers[username]->notifyAllDevices({.opType=1, .file=filename});
    }
}

void clientDownload(int clientSocket, std::string username) {
    if (sendOk(clientSocket) == -1)
        return;
    FileId fileId;
    if (receiveFileId(clientSocket, &fileId) == -1)
        return;
    std::filesystem::path basePath(username.c_str());
    sendFile(clientSocket, basePath/fileId.filename);
}

void handleClient(int clientSocket) {
    Message msg;
    try {
        msg = readMessage(clientSocket);
    } catch (std::runtime_error) {
        fprintf(stderr, "Erro");
        close(clientSocket);
        return;
    }

    //printMsg(&msg);
    if (msg.type == MsgType::MSG_AUTH) {
        sendOk(clientSocket);
    } else {
        sendError(clientSocket, "Expected AUTH msg");
        close(clientSocket);
        return;
    };

    std::string username(msg.payload, msg.payload + msg.len);

    try {
        msg = readMessage(clientSocket);
    } catch (std::runtime_error) {
        fprintf(stderr, "Erro 2");
        close(clientSocket);
        return;
    }
    //printMsg(&msg);

    switch (msg.type) {
        case MsgType::MSG_SYNC:
            handleSync(clientSocket, username);
            break;

        case MsgType::MSG_UPLOAD:
            clientUpload(clientSocket, username);
            break;

        case MsgType::MSG_DOWNLOAD:
            clientDownload(clientSocket, username);
            break;

        default:
            sendError(clientSocket, "Unknown msg type");
            break;
    }
    close(clientSocket);
}

int main(int argc, char *argv[]) {
    int port;
    if (argc != 2) {
        port = 8000;
        //fprintf(stderr, "Usage: server <port>\n");
        //return 1;
    } else {
        port = atoi(argv[1]);
    }

    // Set directory to store user files
    std::filesystem::create_directory(std::filesystem::current_path() / "data");
    std::filesystem::current_path(std::filesystem::current_path() / "data");

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
    addr.sin_port = htons(port);
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
        openConnections.push_back(std::thread(handleClient, clientSocket));
        c++;

        if (c == 10) break; // Just to test server closing
    }

    // Wait for all clients to close their connections
    for (auto &conn: openConnections) {
        conn.join();
    }

    close(sock_fd);

    return 0;
}
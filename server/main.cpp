#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>
#include <string.h>
#include <memory>

#include "Messages.hpp"
#include "handlers/UploadHandler.hpp"
#include "handlers/DownloadHandler.hpp"
#include "handlers/DeleteHandler.hpp"
#include "handlers/ListServerHandler.hpp"
#include "handlers/SyncServerToClientHandler.hpp"
#include "handlers/SyncClientToServerHandler.hpp"
#include "handlers/HeartBeatHandler.hpp"
#include "DeviceManager.hpp"
#include "ReplicaConnection.hpp"
#include "ReplicaManager.hpp"


#define MAX_USER_DEVICES 2

std::map<std::string, DeviceManager*> deviceManagers;
std::mutex userRegisterMutex;
std::shared_ptr<ReplicaConnection> replicaConnectionPtr;
ReplicaManager *replicaManager = nullptr;

void handleClient(int clientSocket, AuthData authData) {
    ClientAuthData clientData = authData.clientData; 
    std::string username(clientData.username, clientData.usernameLen);
    std::filesystem::create_directory(username);

    {
        std::lock_guard<std::mutex> lock(userRegisterMutex);
        if (deviceManagers.find(username) == deviceManagers.end()) {
            deviceManagers[username] = new DeviceManager(username, MAX_USER_DEVICES);
        }
    }

    DeviceManager *userDeviceManager = deviceManagers[username];

    try {
        if (clientData.deviceId == 0) { // Unknown device
            Device device = userDeviceManager->registerDevice();
            clientData.deviceId = device.id;
            std::cout << "User " << username << " connected a new device. Assigned id " << device.id << "\n";
        }
    } catch (TooManyDevices t) {
        try {
            sendError(clientSocket, t.what());
        } catch (BrokenPipe) {}

        return;
    }

    userDeviceManager->connectDevice(clientData.deviceId);

    try {
        authData.clientData = clientData;
        sendAuth(clientSocket, authData);

        std::cout << "User " << username << " authenticated with device " << clientData.deviceId << "\n";

        Device device = userDeviceManager->getDevice(clientData.deviceId);

        while (true) {
            Message msg = receiveMessage(clientSocket);

            switch(msg.type) {
                case MsgType::MSG_UPLOAD:
                    UploadHandler(username, clientSocket, userDeviceManager).run();
                    break;
                case MsgType::MSG_DOWNLOAD:
                    DownloadHandler(username, clientSocket).run();
                    break;
                case MsgType::MSG_DELETE:
                    DeleteHandler(username, clientSocket, userDeviceManager).run();
                    break;
                case MsgType::MSG_LIST_SERVER:
                    ListServerHandler(username, clientSocket).run();
                    break;
                case MsgType::MSG_SYNC_SERVER_TO_CLIENT:
                    SyncServerToClientHandler(username, clientSocket, device).run();
                    break;
                case MsgType::MSG_SYNC_CLIENT_TO_SERVER:
                    SyncClientToServerHandler(username, clientSocket, clientData.deviceId, userDeviceManager).run();
                    break;
                case MsgType::MSG_HEARTBEAT:
                    HeartBeatHandler(clientSocket).run();
                    break;
                default:
                    sendError(clientSocket, "Unrecognized command");
                    break;
            }
        }
    } catch (BrokenPipe) {
        std::cout << "User " << username << " disconnected from device " << clientData.deviceId << "\n";
    } 

    userDeviceManager->disconnectDevice(clientData.deviceId);
}

void handleReplica(int replicaSocket, sockaddr_in replicaAddr, AuthData authData) {
    ReplicaAuthData replicaData = authData.replicaData;
    
    replicaData.ipAddress = replicaAddr.sin_addr.s_addr;
    replicaData.replicaId = authData.replicaData.replicaId;
    authData.replicaData = replicaData;

    std::cout << authData.replicaData.ipAddress << std::endl;

    if (replicaManager == nullptr) {
        replicaManager = new ReplicaManager();
    }
            
    replicaManager->pushReplica(replicaData.replicaId, replicaData.ipAddress, replicaSocket);
    try {
        sendAuth(replicaSocket, authData);
        replicaManager->sendAllReplicas(replicaSocket);
        replicaManager->updateReplica(replicaSocket, replicaData.replicaId);
        std::cout << "Primary pushReplica" << std::endl;
        replicaManager->printReplicas();

        return;

        Message msg = receiveMessage(replicaSocket);

        switch (msg.type) {
            case MsgType::MSG_HEARTBEAT:
                HeartBeatHandler(replicaSocket).run();
                break;
            
            default:
                break;
        }
    } catch (BrokenPipe) {
        char ipString[16];
        inet_ntop(AF_INET, &replicaData.ipAddress, ipString, 16);
        std::cout << "Lost connection to replica " << ipString << "\n";
    }
}


void handleConnection(int remoteSocket, sockaddr_in remoteAddr) {
    try {
        AuthData authData = receiveAuth(remoteSocket);

        switch (authData.type) {
            case AuthType::AUTH_CLIENT:
                handleClient(remoteSocket, authData);
                break;
            case AuthType::AUTH_REPLICA:
                handleReplica(remoteSocket, remoteAddr, authData);
                return;
        }
    } catch (BrokenPipe) {

    }
    close(remoteSocket);
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 5) {
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

    if (argc == 5) {
        int port = std::stoi(argv[3]);
        int deviceId = std::stoi(argv[4]);
        
        replicaConnectionPtr = std::make_shared<ReplicaConnection>(ReplicaConnection(deviceId));
        replicaManager = new ReplicaManager();
        if (!replicaConnectionPtr->setConnection(argv[2], port, replicaManager)) return 1;
    }

 

    std::vector<std::thread> openConnections;
    std::filesystem::path dataDir = std::filesystem::current_path() / "data";
    std::filesystem::create_directories(dataDir);
    std::filesystem::current_path(std::filesystem::current_path() / "data");
    int c = 0;
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(sock_fd, (sockaddr*)&clientAddr, &clientAddrSize);
        openConnections.push_back(std::thread(handleConnection, clientSocket, clientAddr));
    }

    // Wait for all clients to close their connections
    for (auto &conn: openConnections) {
        conn.join();
    }

    close(sock_fd);

    return 0;
}

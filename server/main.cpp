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
#include "handlers/ListServerHandler.hpp"
#include "handlers/SyncServerToClientHandler.hpp"
#include "handlers/SyncClientToServerHandler.hpp"
#include "handlers/HeartBeatHandler.hpp"
#include "DeviceManager.hpp"

#define MAX_USER_DEVICES 2

std::map<std::string, DeviceManager*> deviceManagers;
std::mutex userRegisterMutex;

void handleConnection(int remoteSocket) {
    DeviceManager *userDeviceManager = nullptr;
    int deviceId = -1;
    std::string username;

    try {
        AuthData authData = receiveAuth(remoteSocket);
        username = std::string(authData.username, authData.usernameLen);
        std::filesystem::create_directory(username);

        {
            std::lock_guard<std::mutex> lock(userRegisterMutex);
            if (deviceManagers.find(username) == deviceManagers.end()) {
                deviceManagers[username] = new DeviceManager(username, MAX_USER_DEVICES);
            }
        }

        userDeviceManager = deviceManagers[username];
    
        if (authData.deviceId == 0) { // Unknown device
            Device device = userDeviceManager->registerDevice();
            authData.deviceId = device.id;
            std::cout << "User " << username << " connected a new device. Assigned id " << device.id << "\n";
        }
        deviceId = authData.deviceId;
        userDeviceManager->connectDevice(deviceId);
        sendAuth(remoteSocket, authData);

        std::cout << "User " << username << " authenticated with device " << deviceId << "\n";

        Device device = userDeviceManager->getDevice(deviceId);

        while (true) {
            Message msg = receiveMessage(remoteSocket);

            switch(msg.type) {
                case MsgType::MSG_UPLOAD:
                    UploadHandler(username, remoteSocket, userDeviceManager).run();
                    break;
                case MsgType::MSG_DOWNLOAD:
                    DownloadHandler(username, remoteSocket).run();
                    break;
                case MsgType::MSG_DELETE:
                    DeleteHandler(username, remoteSocket, userDeviceManager).run();
                    break;
                case MsgType::MSG_LIST_SERVER:
                    ListServerHandler(username, remoteSocket).run();
                    break;
                case MsgType::MSG_SYNC_SERVER_TO_CLIENT:
                    SyncServerToClientHandler(username, remoteSocket, device).run();
                    break;
                case MsgType::MSG_SYNC_CLIENT_TO_SERVER:
                    SyncClientToServerHandler(username, remoteSocket, deviceId, userDeviceManager).run();
                    break;
                case MsgType::MSG_HEARTBEAT:
                    HeartBeatHandler(remoteSocket).run();
                    break;
                default:
                    sendError(remoteSocket, "Unrecognized command");
                    break;
            }
        }
    } catch (BrokenPipe) {
        std::cout << "User " << username << " disconnected from device " << deviceId << "\n";
    } catch (TooManyDevices t) {
        sendError(remoteSocket, t.what());
    } 

    if (userDeviceManager != nullptr && deviceId != -1) {
        userDeviceManager->disconnectDevice(deviceId);
    }
    close(remoteSocket);
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
    std::filesystem::path dataDir = std::filesystem::current_path() / "data";
    std::filesystem::create_directories(dataDir);
    std::filesystem::current_path(std::filesystem::current_path() / "data");
    int c = 0;
    while (true) {
        int remoteSocket = accept(sock_fd, nullptr, nullptr);
        openConnections.push_back(std::thread(handleConnection, remoteSocket));
    }

    // Wait for all clients to close their connections
    for (auto &conn: openConnections) {
        conn.join();
    }

    close(sock_fd);

    return 0;
}

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
#include <algorithm>

#include "Messages.hpp"
#include "handlers/UploadHandler.hpp"
#include "handlers/DownloadHandler.hpp"
#include "handlers/DeleteHandler.hpp"
#include "handlers/ListServerHandler.hpp"
#include "handlers/SyncServerToClientHandler.hpp"
#include "handlers/SyncClientToServerHandler.hpp"
#include "handlers/HeartBeatHandler.hpp"
#include "handlers/ElectionHandler.hpp"
#include "handlers/ElectedHandler.hpp"
#include "handlers/ReplicaConnectionHandler.hpp"
#include "DeviceManager.hpp"
#include "ReplicaThread.hpp"
#include "ReplicaManager.hpp"
#include "ElectionManager.hpp"
#include "BinderManager.hpp"
#include "utils.hpp"

#define MAX_USER_DEVICES 2

std::map<std::string, DeviceManager*> deviceManagers;
std::mutex userRegisterMutex;
ReplicaManager replicaManager;
ElectionManager *electionManager;
BinderManager *binderManager;

int myId;


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

        close(clientSocket);

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
                    UploadHandler(username, clientSocket, userDeviceManager, &replicaManager).run();
                    break;
                case MsgType::MSG_DOWNLOAD:
                    DownloadHandler(username, clientSocket).run();
                    break;
                case MsgType::MSG_DELETE:
                    DeleteHandler(username, clientSocket, userDeviceManager, &replicaManager).run();
                    break;
                case MsgType::MSG_LIST_SERVER:
                    ListServerHandler(username, clientSocket).run();
                    break;
                case MsgType::MSG_SYNC_SERVER_TO_CLIENT:
                    SyncServerToClientHandler(username, clientSocket, device).run();
                    break;
                case MsgType::MSG_SYNC_CLIENT_TO_SERVER:
                    SyncClientToServerHandler(username, clientSocket, clientData.deviceId, userDeviceManager, &replicaManager).run();
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
    close(clientSocket);
}

void handleReplica(int replicaSocket, sockaddr_in replicaAddr, AuthData authData) {
    ReplicaAuthData replicaData = authData.replicaData;
    
    replicaData.replicaAddr = {
        .ip=replicaAddr.sin_addr.s_addr,
        .port=authData.replicaData.replicaAddr.port
    };
    replicaData.replicaId = authData.replicaData.replicaId;
    authData.replicaData = replicaData;
   
    try {
        sendAuth(replicaSocket, authData);

        Message msg = receiveMessage(replicaSocket);

        switch (msg.type) {
            case MsgType::MSG_HEARTBEAT:
                HeartBeatHandler(replicaSocket).run();
                break;

            case MsgType::MSG_REPLICATION:
                ReplicaConnectionHandler(replicaSocket, replicaData.replicaId, replicaData.replicaAddr, &replicaManager).run();
                // Early return to skip closing the socket. We need to keep it open to send updates to the replica
                return;

            case MsgType::MSG_ELECTION:
                ElectionHandler(replicaSocket, myId, electionManager, &replicaManager).run();
                break;

            case MsgType::MSG_ELECTED:
                ElectedHandler(replicaSocket, myId, &replicaManager, electionManager).run();
                break;

            default:
                break;
        }
    } catch (BrokenPipe) {
        std::cout << "Server " << replicaData.replicaAddr << " disconnected\n";
    }
    close(replicaSocket);
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
}


void startReplicationThread(ServerAddress primaryAddr, uint16_t port){
    ReplicaThread replicaThread;
    replicaThread.run(&replicaManager, myId, port, primaryAddr);
}

void electionMonitor(ServerAddress primaryAddr, uint16_t port) {
    AuthData authData = {
        .type=AuthType::AUTH_REPLICA,
        .replicaData={
            .replicaAddr={
                .port=port
            },
            .replicaId=myId,
        }
    };

    electionManager = new ElectionManager(binderManager, myId, 0, primaryAddr);

    while (electionManager->getLeader() != myId) {
        try {
            int primarySock = openSocketTo(electionManager->getLeaderAddress());
            if (primarySock == -1) {
                std::cout << "Cannot connect to primary server\n";
                return;
            }
            sendAuth(primarySock, authData);
            receiveAuth(primarySock);

            sendMessage(primarySock, MsgType::MSG_HEARTBEAT, nullptr, 0);
            waitConfirmation(primarySock);

            while (true) {
                waitHeartbeat(primarySock, 1000);
            }
        } catch (BrokenPipe) {
            std::cout << "Primary died. Starting election.\n";

            ServerAddress nextServerAddr = replicaManager.getNextReplica();
            std::cout << "Connecting to next replica " << nextServerAddr << "\n";
            int nextServer = openSocketTo(nextServerAddr);
            if (nextServer == -1) {
                std::cout << "Cannot connect to next server on ring\n";
                return;
            }

            try {
                sendAuth(nextServer, authData);
                receiveAuth(nextServer);

                sendMessage(nextServer, MsgType::MSG_ELECTION, nullptr, 0);
                waitConfirmation(nextServer);

                Ballot ballot = {
                    .address=replicaManager.getAddress(),
                    .id=myId
                };
                sendBallot(nextServer, ballot);
                waitConfirmation(nextServer);
            } catch (BrokenPipe) {
                std::cout << "Caiu\n";
            } catch (ErrorReply e) {
                std::cout << e.what() << "\n";
            } catch (UnexpectedMsgType e) {
                std::cout << e.what() << "\n";
            }
        }
        std::cout << "Waiting election finish\n";
        electionManager->waitElectionEnd();

        std::cout << "New leader set to " << electionManager->getLeaderAddress() << "\n";

        if (electionManager->getLeader() != myId) {
            std::cout << "Connecting to the new leader\n";
            startReplicationThread(electionManager->getLeaderAddress(), port);
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 7) {
        fprintf(stderr, "Usage: server <server-port> <binder-ip> <binder-port> [<primary-ip> <primary-port> <id>]\n");
        return 1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock_fd == -1) {
        fprintf(stderr, "Error on creating socket\n");
        return 1;
    }
    
    int optVal = 1;
    setsockopt(sock_fd, 1, SO_REUSEADDR, &optVal, sizeof(optVal));

    uint16_t myPort = htons(atoi(argv[1]));

    // Binding do socket na porta 8000
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = myPort;
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

    ServerAddress binderAddress;
    inet_pton(AF_INET, argv[2], &binderAddress.ip);
    binderAddress.port = htons(atoi(argv[3]));
    std::cout << "Connecting to binder at " << binderAddress << "\n";
    binderManager = new BinderManager(binderAddress); 
    
    if (argc == 7) {
        uint16_t primaryPort = htons(atoi(argv[5]));    
        myId = std::stoi(argv[6]);
        ServerAddress primaryAddr;
        inet_pton(AF_INET, argv[4], &primaryAddr.ip);
        primaryAddr.port = primaryPort;
        startReplicationThread(primaryAddr, myPort);
        std::thread(electionMonitor, primaryAddr, myPort).detach();
    } else {
        ServerAddress myAddr;
        myAddr.port = htons(atoi(argv[1]));
        binderManager->notifyBinder(myAddr);
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

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
#include "ReplicaConnection.hpp"
#include "ReplicaManager.hpp"
#include "ElectionManager.hpp"
#include "utils.hpp"

#define MAX_USER_DEVICES 2

std::map<std::string, DeviceManager*> deviceManagers;
std::mutex userRegisterMutex;
std::shared_ptr<ReplicaConnection> replicaConnectionPtr;
ReplicaManager *replicaManager = nullptr;
ElectionManager *electionManager;

ServerAddress myAddress = {.ip=16777343};
std::vector<ServerAddress> replicas;
int myId;

ServerAddress findNextServerAddr(){
    for (int i = 0; i < replicas.size(); i++) {
        if (replicas[i] == myAddress) {
            int nextIdx = (i + 1) % replicas.size();
            std::cout << "Next server idx: " << nextIdx << std::endl;
            return replicas[nextIdx];
        }
    }
        std::cout << "aaaa\n";
}

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
    std::cout << "Replica connected\n";
    ReplicaAuthData replicaData = authData.replicaData;

    if(replicaManager == nullptr) {
        replicaManager = new ReplicaManager();
    }
    
    replicaData.ipAddress = replicaAddr.sin_addr.s_addr;
    replicaData.replicaId = authData.replicaData.replicaId;
    authData.replicaData = replicaData;


    std::cout << authData.replicaData.ipAddress << std::endl;
            
    try {
        sendAuth(replicaSocket, authData);

        Message msg = receiveMessage(replicaSocket);

        switch (msg.type) {
            case MsgType::MSG_HEARTBEAT:
                HeartBeatHandler(replicaSocket).run();
                break;
            case MsgType::MSG_UPDATE_TYPE:
                ReplicaConnectionHandler(replicaSocket, replicaData.replicaId, replicaData.ipAddress, replicaManager).run();
                break;

            case MsgType::MSG_ELECTION:
                ElectionHandler(replicaSocket, myAddress, myId, findNextServerAddr(), electionManager).run();
                break;

            case MsgType::MSG_ELECTED:
                ElectedHandler(replicaSocket, myId, findNextServerAddr(), electionManager).run();
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


void electionMonitor(ServerAddress primaryAddr) {
    AuthData authData = {
        .type=AuthType::AUTH_REPLICA,
    };

    electionManager = new ElectionManager(0, primaryAddr);
    
    while (electionManager->getLeader() != myId) {
        try {
            int primarySock = openSocketTo(primaryAddr);
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

            ServerAddress nextServerAddr = findNextServerAddr();
            std::cout << "Next Server: IP="<<nextServerAddr.ip << " Port="<<nextServerAddr.port << std::endl;
            int nextServer = openSocketTo(nextServerAddr);
            if (nextServer == -1) {
                std::cout << "Cannot connect to next server on ring\n";
                return;
            }
            
            try {
                electionManager->markParticipation();
                sendAuth(nextServer, authData);
                receiveAuth(nextServer);

                sendMessage(nextServer, MsgType::MSG_ELECTION, nullptr, 0);
                waitConfirmation(nextServer);

                Ballot ballot = {
                    .address=myAddress,
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
        electionManager->waitElectionEnd();
        // Wait for the election to end 
    }
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

        
    if (argc == 5) {
        uint16_t port = atoi(argv[3]);
        myAddress.port = htons(atoi(argv[1]));
        std::cout << myAddress.port << "\n";
        myId = std::stoi(argv[4]);
        
        ServerAddress primaryAddr;

        inet_pton(AF_INET, argv[2], &primaryAddr.ip);
        primaryAddr.port = htons(port);

        // Hard coded for now...
        replicas.push_back((ServerAddress){.ip=16777343, .port=htons(8001)});
        replicas.push_back((ServerAddress){.ip=16777343, .port=htons(8002)});

        std::cout << "Starting election monitor\n";
        std::thread(electionMonitor, primaryAddr).detach();
    }
 
    std::cout << "Server listening on port " << argv[1] << "\n";
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

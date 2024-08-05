#pragma once

#include "Messages.hpp"
#include <thread>


class ReplicaConnection {
    private:
        int replicaSock = -1;
        int replicaId;
        uint32_t replicaIpAddress;
        std::thread replicaThread;

        void createSocket(int &socketDescr, std::string ip, int port);
        bool replicaAuth(int &socketDescr, int replicaId);
        void getServerData(int &socketDescr);

    public:
        ReplicaConnection(int replicaId);
        bool setConnection(std::string ip, int port);
};

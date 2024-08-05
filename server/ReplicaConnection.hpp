#pragma once

#include "Messages.hpp"
#include "ReplicaThread.hpp"


class ReplicaConnection {
    std::shared_ptr<ReplicaThread> replicaThread;

    private:
        int replicaSock = -1;
        int replicaId;
        uint32_t replicaIpAddress;

        void createSocket(int &socketDescr, std::string ip, int port);
        bool replicaAuth(int &socketDescr, int replicaId);
        void runReplicaThread(int &socketDescr);

    public:
        ReplicaConnection(int replicaId);
        bool setConnection(std::string ip, int port);
};

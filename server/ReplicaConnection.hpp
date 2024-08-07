#pragma once

#include "Messages.hpp"
#include "ReplicaThread.hpp"
#include "ReplicaManager.hpp"


class ReplicaConnection {
    std::shared_ptr<ReplicaThread> replicaThread;

    private:
        int replicaSock = -1;
        int replicaId;
        uint32_t replicaIpAddress;

        void createSocket(int &socketDescr, std::string ip, int port);
        bool replicaAuth(int &socketDescr, int replicaId);
        void runReplicaThread(int &socketDescr, ReplicaManager* replicaManager);
        void initializeReplicaManager(int &socketDescr, ReplicaManager* replicaManager);

    public:
        ReplicaConnection(int replicaId);
        bool setConnection(std::string ip, int port, ReplicaManager* replicaManager);
};

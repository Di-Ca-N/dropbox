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

        void createSocket(int &socketDescr, ServerAddress primaryAddr);
        bool replicaAuth(int &socketDescr, int replicaId, uint16_t port);
        void runReplicaThread(int &socketDescr, ReplicaManager* replicaManager);
        void initializeReplicaManager(int &socketDescr, ReplicaManager* replicaManager);
        bool createUpdateType(int &socketDescr);

    public:
        ReplicaConnection(int replicaId);
        bool setConnection(ServerAddress primaryAddr, int myPort, ReplicaManager* replicaManager);
};

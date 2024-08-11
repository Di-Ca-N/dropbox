#pragma once

#include <thread>
#include <Messages.hpp>
#include "ReplicaManager.hpp"


class ReplicaThread {
private:
    ReplicaData replicaData;
    std::thread replicaThread;

    void getServerUpdates(ReplicaManager* replicaManager,
                          int replicaId, uint16_t port,
                          ServerAddress primaryAddr);
    void initializeReplicaManager(int socketDescr, ReplicaManager* replicaManager);
    void getNewReplica(int socketDescr, ReplicaManager* replicaManager);
    void removeReplica(int socketDescr, ReplicaManager* replicaManager);

   public:
    void run(ReplicaManager* replicaManager, int replicaId,
             uint16_t port, ServerAddress primaryAddr);
};
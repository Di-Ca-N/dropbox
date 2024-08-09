#pragma once

#include <thread>
#include <Messages.hpp>
#include "ReplicaManager.hpp"


class ReplicaThread {
private:
    ReplicaData replicaData;
    std::thread replicaThread;
    void getServerUpdates(int socketDescr, ReplicaManager* replicaManager);
    void getNewReplica(int socketDescr, ReplicaManager* replicaManager);
    void removeReplica(int socketDescr, ReplicaManager* replicaManager);

public:
    void run(int &socketDescr, ReplicaManager* replicaManager);
    ~ReplicaThread();
};
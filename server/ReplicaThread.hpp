#pragma once

#include <thread>
#include <Messages.hpp>
#include "ReplicaManager.hpp"


class ReplicaThread {
private:
    std::thread replicaThread;
    void getServerUpdates(int socketDescr);

public:
    void run(int &socketDescr, ReplicaManager* replicaManager);
    ~ReplicaThread();
};
#pragma once

#include <thread>
#include <Messages.hpp>

class ReplicaThread {
private:
    std::thread replicaThread;
    void getServerUpdates(int socketDescr);

public:
    void run(int &socketDescr);
    ~ReplicaThread();
};
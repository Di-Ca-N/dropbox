#include <iostream>
#include <cstring> // For strerror
#include <unistd.h> // For close

#include "ReplicaThread.hpp"

void ReplicaThread::getServerUpdates(int socketDescr) {
    while (true) {
        std::cout << "oi" << std::endl;
    }
    close(socketDescr);
}

void ReplicaThread::run(int &socketDescr) {
    // Start the thread to get server updates
    replicaThread = std::thread(&ReplicaThread::getServerUpdates, this, socketDescr);
}

ReplicaThread::~ReplicaThread() {
    if (replicaThread.joinable()) {
        replicaThread.join(); // Wait for the thread to finish before destroying the object
    }
}
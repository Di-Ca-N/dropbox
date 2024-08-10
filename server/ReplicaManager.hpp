#pragma once

#include <map>
#include <cstdint>  
#include <vector>

#include "Messages.hpp"

struct Replica {
    int replicaId;
    ServerAddress replicaAddr;
    int socketDescr;
};

class ReplicaManager {
    private:
        std::map<int, Replica> replicas;
        ReplicaData replicaData;
        void sendReplica(int socketDescr, int replicaId);
    
    public: 
        void pushReplica(int replicaId, ServerAddress addr, int socketDescr);
        void popReplica(int replicaId);
        void updateReplica(int replicaId, UpdateType updateType);
        void removeReplica(int replicaId, UpdateType updateType);
        void sendAllReplicas(int &socketDescr);
        void printReplicas() const;
        std::vector<ServerAddress> getReplicas();
        ServerAddress getNextReplica(ServerAddress currentAddress);
};

#pragma once

#include <map>
#include <cstdint>  

#include "Messages.hpp"

struct Replica {
    int replicaId;
    uint32_t replicaIp;
    int socketDescr;
};

class ReplicaManager {
    private:
        std::map<int, Replica> replicas;
        ReplicaData replicaData;
        void sendReplica(int socketDescr, int replicaId);
    
    public: 
        void pushReplica(int replicaId, uint32_t replicaIp, int socketDescr);
        void popReplica(int replicaId);
        void updateReplica(int replicaId, UpdateType updateType);
        void removeReplica(int replicaId, UpdateType updateType);
        void sendAllReplicas(int &socketDescr);
        void printReplicas() const;
};

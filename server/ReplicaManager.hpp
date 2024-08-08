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
    
    public: 
        void pushReplica(int replicaId, uint32_t replicaIp, int socketDescr);
        void popReplica(int replica);
        void updateReplica(int &socketDescr, int replicaId);
        void sendAllReplicas(int &socketDescr);
        void sendNewReplica(int socketDescr, int replicaId);
        void printReplicas() const;
};

#include "Handler.hpp"
#include "../ReplicaManager.hpp"
#include "Messages.hpp"
#include <iostream>

class ReplicaConnectionHandler : public Handler  {
    private:
        int replicaId;
        uint32_t replicaIp;
        int replicaSock;
        ReplicaManager* replicaManager;

        void UpdateConnectionStart();
    
    public:
        ReplicaConnectionHandler(int socketDescr, int replicaId, uint32_t replicaIp, ReplicaManager* replicaManager);
        void run();
};
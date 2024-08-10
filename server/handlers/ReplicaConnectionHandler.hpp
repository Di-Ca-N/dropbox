#include "Handler.hpp"
#include "../ReplicaManager.hpp"
#include "Messages.hpp"
#include <iostream>

class ReplicaConnectionHandler : public Handler  {
    private:
        int replicaId;
        ServerAddress replicaAddr;
        int replicaSock;
        ReplicaManager* replicaManager;

        void UpdateConnectionStart();
    
    public:
        ReplicaConnectionHandler(int socketDescr, int replicaId, ServerAddress replicaAddr, ReplicaManager* replicaManager);
        void run();
};
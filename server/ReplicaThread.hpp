#pragma once

#include <thread>
#include <Messages.hpp>
#include "ReplicaManager.hpp"


class ReplicaThread {
private:
    ReplicaData replicaData;
    std::thread replicaThread;
    std::thread syncThread;
    
    void getServerUpdates(ReplicaManager* replicaManager,
                          int replicaId, uint16_t port,
                          ServerAddress primaryAddr);
    void initializeReplicaManager(int socketDescr, ReplicaManager* replicaManager);
    void createDir(int socketDescr);
    void getDirFiles(int socketDescr, std::string dirName);
    void getNewReplica(int socketDescr, ReplicaManager* replicaManager);
    void removeReplica(int socketDescr, ReplicaManager* replicaManager);
    void handleFileOp(int socketDescr);
    void fileUpdate(int socketDescr);
    void handleModify(int socketDescr);
    void handleFileDelete(int socketDescr);

   public:
    void run(ReplicaManager* replicaManager, int replicaId, uint16_t port,
             ServerAddress primaryAddr);
};
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
        int countDirectories(const std::filesystem::path& baseDir);
        int countFiles(const std::filesystem::path& baseDir);
        void createDir(int &socketDescr, std::string dirName);
        void sendFile(int &socketDescr, int fileNum, const std::filesystem::path& filePath);
    
    public: 
        void pushReplica(int replicaId, ServerAddress addr, int socketDescr);
        void popReplica(int replicaId);
        void updateReplica(int replicaId, UpdateType updateType);
        void removeReplica(int replicaId, UpdateType updateType);
        void sendAllReplicas(int &socketDescr);
        void sendAllFiles(int &socketDescr);
        void printReplicas() const;
        void notifyAllReplicas(FileOperation op, std::string username);
        std::vector<ServerAddress> getReplicas();
        ServerAddress getNextReplica(ServerAddress currentAddress);
};

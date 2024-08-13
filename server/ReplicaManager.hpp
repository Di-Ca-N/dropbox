#pragma once

#include <map>
#include <cstdint>  
#include <vector>
#include <mutex>

#include "Messages.hpp"

struct Replica {
    int replicaId;
    ServerAddress replicaAddr;
    int socketDescr;
};

class ReplicaManager {
    private:
        std::mutex mutex;

        std::map<int, Replica> replicas;
        ReplicaData replicaData;
        ServerAddress myAddress;

        void sendReplica(int socketDescr, int replicaId);
        int countDirectories(const std::filesystem::path& baseDir);
        int countFiles(const std::filesystem::path& baseDir);
        void createDir(int &socketDescr, std::string dirName);
        void sendFile(int &socketDescr, int fileNum, const std::filesystem::path& filePath);
        void handleFileModify(int &socketDescr, std::string filename, std::string username);
        void handleFileDelete(int socketDescr, std::string filename,
                              std::string username);
        std::vector<ServerAddress> getReplicas();

    public: 
        void pushReplica(int replicaId, ServerAddress addr, int socketDescr);
        void popReplica(int replicaId);
        void clearReplicas();
        void updateReplica(int replicaId, UpdateType updateType);
        void removeReplica(int replicaId, UpdateType updateType);
        void sendAllReplicas(int &socketDescr);
        void sendAllFiles(int &socketDescr);
        void printReplicas() const;
        void setAddress(ServerAddress address);
        ServerAddress getAddress();

        void notifyAllReplicas(FileOperation op, std::string username);
        ServerAddress getNextReplica();
};

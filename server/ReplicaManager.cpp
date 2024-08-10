#include "ReplicaManager.hpp"
#include <iostream>

void ReplicaManager::pushReplica(int replicaId, ServerAddress replicaAddr, int socketDescr) {
    replicas[replicaId] = {replicaId, replicaAddr, socketDescr};
}

void ReplicaManager::popReplica(int replicaId) {
    if(replicas.find(replicaId) != replicas.end()) {
        replicas.erase(replicaId);
    }
}

std::vector<ServerAddress> ReplicaManager::getReplicas() {
    std::vector<ServerAddress> replicaAddrs;
    for (auto &[id, replica] : replicas) {
        replicaAddrs.push_back(replica.replicaAddr);
    }
    return replicaAddrs;
}

ServerAddress ReplicaManager::getNextReplica(ServerAddress currentAddress) {
    std::vector<ServerAddress> replicaAddrs = this->getReplicas();
    for (int i = 0; i < replicaAddrs.size(); i++) {
        if (replicaAddrs[i] == currentAddress) {
            int nextIdx = (i + 1) % replicaAddrs.size();
            return replicaAddrs[nextIdx];
        }
    }
}

void ReplicaManager::updateReplica(int replicaId, UpdateType updateType) {
    for (const auto& pair : replicas) {
        const Replica& replica = pair.second;
        try {
            if(replicaId != replica.replicaId) {
                sendUpdateType(replica.socketDescr, updateType);
                sendReplica(replica.socketDescr, replicaId);
            }
           
        } catch (UnexpectedMsgType) {
            std::cout << "Unexpected response.\n";
            return;
        } catch (ErrorReply e) {
            std::cout << "Error: " << e.what() << "\n";
            return;
        }
    }
}

void ReplicaManager::removeReplica(int replicaId, UpdateType updateType) {
    for (const auto& pair : replicas) {
        const Replica& replica = pair.second;
        try {
            if(replicaId != replica.replicaId) {
                sendUpdateType(replica.socketDescr, updateType);
                sendReplicaId(replica.socketDescr, replicaId);
            }
           
        } catch (UnexpectedMsgType) {
            std::cout << "Unexpected response.\n";
            return;
        } catch (ErrorReply e) {
            std::cout << "Error: " << e.what() << "\n";
            return;
        }
    }
}

void ReplicaManager::sendReplica(int socketDescr, int replicaId) {
  
    replicaData.replicaId = replicaId;
    replicaData.replicaAddr = replicas[replicaId].replicaAddr;    
    replicaData.socketDescr = replicas[replicaId].socketDescr;
    
    try {
        sendReplicaData(socketDescr, replicaData);
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }

}


void ReplicaManager::sendAllReplicas(int &socketDescr) {
    try {
        sendNumFiles(socketDescr, replicas.size());
        waitConfirmation(socketDescr);
        for (const auto& replica : replicas) {
            replicaData.replicaId = replica.second.replicaId;
            replicaData.replicaAddr = replica.second.replicaAddr;
            replicaData.socketDescr = replica.second.socketDescr;
            sendReplicaData(socketDescr, replicaData);
        }
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}


void ReplicaManager::printReplicas() const {
    for (const auto& pair : replicas) {
        const Replica& replica = pair.second;
        std::cout << "Device ID: " << replica.replicaId 
                  << ", Address: " << replica.replicaAddr 
                  << ", Socket Descriptor: " << replica.socketDescr 
                  << std::endl;
    }
}

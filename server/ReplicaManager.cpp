#include "ReplicaManager.hpp"
#include <iostream>

void ReplicaManager::pushReplica(int replicaId, uint32_t replicaIp, int socketDescr) {
    replicas[replicaId] = {replicaId, replicaIp, socketDescr};
}

void ReplicaManager::popReplica(int replicaId) {
    if(replicas.find(replicaId) != replicas.end()) {
         replicas.erase(replicaId);
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
    replicaData.replicaIp = replicas[replicaId].replicaIp;        
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
            replicaData.replicaIp = replica.second.replicaIp;
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
                  << ", Device IP: " << replica.replicaIp 
                  << ", Socket Descriptor: " << replica.socketDescr 
                  << std::endl;
    }
}

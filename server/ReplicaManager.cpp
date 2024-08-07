#include "ReplicaManager.hpp"
#include <iostream>

void ReplicaManager::pushReplica(int replicaId, uint32_t replicaIp, int socketDescr) {
    replicas[replicaId] = {replicaId, replicaIp, socketDescr};
}

void ReplicaManager::popReplica(int deviceId) {
    replicas.erase(deviceId);
}


void ReplicaManager::updateReplica(int replicaId, uint32_t replicaIp, int socketDescr) {
    for (const auto& pair : replicas) {
        try {
            const Replica& replica = pair.second;
            std::cout << "Replica ID: " << replica.replicaId 
                  << ", Socket Descriptor: " << replica.socketDescr << std::endl;
            sendUpdateType(replica.socketDescr, UpdateType::UPDATE_CONNECTION);
            waitConfirmation(replica.socketDescr);
        } catch (UnexpectedMsgType) {
            std::cout << "Unexpected response.\n";
            return;
        } catch (ErrorReply e) {
            std::cout << "Error: " << e.what() << "\n";
            return;
        }
    }
}

void ReplicaManager::sendReplica(int &socketDescr) {
    ReplicaData replicaData;

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

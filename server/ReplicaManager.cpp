#include "ReplicaManager.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstring>


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
    throw std::runtime_error("Current address is not a registered replica");
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

void ReplicaManager::sendAllFiles(int &socketDescr) {
    std::filesystem::path baseDir("./");
    std::string dirName;
    int countDir, countFile = 0;

    try {
        countDir = countDirectories(baseDir);
        sendNumFiles(socketDescr, countDir);
        waitConfirmation(socketDescr);

        if (std::filesystem::exists(baseDir) && std::filesystem::is_directory(baseDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(baseDir)) {
                if (entry.is_directory()) {
                    dirName = entry.path().filename().string();
                    createDir(socketDescr, dirName);

                    countFile = countFiles(entry.path());
                    sendFile(socketDescr, countFile, entry.path());
                }
            }
        } else {
            std::cerr << "O caminho especificado não é um diretório ou não existe." << std::endl;
        }
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}

void ReplicaManager::sendFile(int &socketDescr, int fileNum, const std::filesystem::path& filePath) {
    FileId fileId;

    try {
        sendNumFiles(socketDescr, fileNum);
        waitConfirmation(socketDescr);

        for (const auto& fileEntry : std::filesystem::directory_iterator(filePath)) {
            if (fileEntry.is_regular_file()) {                
                std::cout << "  Arquivo: " << fileEntry.path() << std::endl;
                fileId = getFileId(fileEntry.path());
                sendFileId(socketDescr, fileId);
                waitConfirmation(socketDescr);
            }
        }
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}

void ReplicaManager::createDir(int &socketDescr, std::string dirName) {
   DirData dirData;
   
   try {
    dirName.copy(dirData.dirName, MAX_DIRNAME);
    dirData.dirnameLen = dirName.length();
    sendDirName(socketDescr, dirData);
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
} 


int ReplicaManager::countDirectories(const std::filesystem::path& baseDir) {
    int dirCount = 0;

    try {
        if (std::filesystem::exists(baseDir) && std::filesystem::is_directory(baseDir)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(baseDir)) {
                if (entry.is_directory()) {
                    ++dirCount;
                }
            }
        } else {
            std::cerr << "Invalid directory path: " << baseDir << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return dirCount;
}

int ReplicaManager::countFiles(const std::filesystem::path& baseDir) {
    int filesCount = 0;

    try {
        if (std::filesystem::exists(baseDir) && std::filesystem::is_directory(baseDir)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(baseDir)) {
                if (entry.is_regular_file()) {
                    ++filesCount;
                }
            }
        } else {
            std::cerr << "Invalid directory path: " << baseDir << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return filesCount;
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

void ReplicaManager::notifyAllReplicas(FileOperation op, std::string username) {
    for (auto &replica : replicas) {
        // notifica a réplica

    }
}

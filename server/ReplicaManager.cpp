#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstring>

#include "ReplicaManager.hpp"
#include "utils.hpp"


void ReplicaManager::pushReplica(int replicaId, ServerAddress replicaAddr, int socketDescr) {
    std::lock_guard<std::mutex> lock(mutex);
    replicas[replicaId] = {replicaId, replicaAddr, socketDescr};
}

void ReplicaManager::popReplica(int replicaId) {
    std::lock_guard<std::mutex> lock(mutex);
    if(replicas.find(replicaId) != replicas.end()) {
        replicas.erase(replicaId);
    }
}

void ReplicaManager::clearReplicas() {
    std::lock_guard<std::mutex> lock(mutex);
    replicas.clear();
}

std::vector<ServerAddress> ReplicaManager::getReplicas() {
    std::vector<ServerAddress> replicaAddrs;
    for (auto &[id, replica] : replicas) {
        replicaAddrs.push_back(replica.replicaAddr);
    }
    return replicaAddrs;
}

ServerAddress ReplicaManager::getNextReplica() {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<ServerAddress> replicaAddrs = this->getReplicas();
    for (int i = 0; i < replicaAddrs.size(); i++) {
        if (replicaAddrs[i] == myAddress) {
            int nextIdx = (i + 1) % replicaAddrs.size();
            return replicaAddrs[nextIdx];
        }
    }
    throw std::runtime_error("Current address is not a registered replica");
}

void ReplicaManager::updateReplica(int replicaId, UpdateType updateType) {
    std::lock_guard<std::mutex> lock(mutex);

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
        waitConfirmation(socketDescr);
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
                fileId = getFileId(fileEntry.path());
                sendFileId(socketDescr, fileId);
                waitConfirmation(socketDescr);

                std::ifstream file(fileEntry.path(), std::ios::binary);
                sendFileData(socketDescr, fileId.totalBlocks, file);
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

void ReplicaManager::notifyAllReplicas(FileOperation op, std::string username) {
    std::lock_guard<std::mutex> lock(mutex);

    std::string fileName(op.filename, op.filenameSize);
    
    for (auto &replica : replicas) {
        std::cout << "Sending info to replica " << replica.first << "\n";
        int replicaSock = replica.second.socketDescr;
        sendUpdateType(replicaSock, UpdateType::UPDATE_FILE_OP);
        waitConfirmation(replicaSock);

        switch (op.type) {
            case FileOpType::FILE_MODIFY:
                handleFileModify(replicaSock, fileName, username);
                break;
            case FileOpType::FILE_DELETE:
                handleFileDelete(replicaSock, fileName, username);
                break;
            default:
                break;
        }
    }
}

void ReplicaManager::handleFileModify(int &socketDescr, std::string filename, std::string username) {
    std::cout << "Sending update on file " << filename << " of " << username << "\n";

    sendFileOperation(socketDescr, FileOpType::FILE_MODIFY);
    waitConfirmation(socketDescr);

    std::filesystem::path baseDir(username.c_str());
    std::string dirName = baseDir.filename().string();
    DirData dirData;
    dirName.copy(dirData.dirName, MAX_DIRNAME);
    dirData.dirnameLen = dirName.length();
    sendDirName(socketDescr, dirData);
    waitConfirmation(socketDescr);

    std::filesystem::path filepath = baseDir / filename;
    
    FileId fid;
    buildFileIdFromPath(filepath, &fid);
    sendFileId(socketDescr, fid);
    waitConfirmation(socketDescr);

    std::ifstream file(filepath, std::ios::binary);
    sendFileData(socketDescr, fid.totalBlocks, file);
    waitConfirmation(socketDescr);
}

void ReplicaManager::handleFileDelete(int socketDescr, std::string filename, std::string username) {
    std::cout << "Sending deletion on file " << filename << " of " << username << "\n";
    sendFileOperation(socketDescr, FileOpType::FILE_DELETE);
    waitConfirmation(socketDescr);

    std::filesystem::path baseDir(username.c_str());
    std::string dirName = baseDir.filename().string();
    DirData dirData;
    dirName.copy(dirData.dirName, MAX_DIRNAME);
    dirData.dirnameLen = dirName.length();
    sendDirName(socketDescr, dirData);
    waitConfirmation(socketDescr);

    std::filesystem::path filepath = baseDir / filename;

    FileId fid;
    filename.copy(fid.filename, MAX_FILENAME);
    fid.filenameSize = filename.size();
    sendFileId(socketDescr, fid);
    waitConfirmation(socketDescr);
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

void ReplicaManager::setAddress(ServerAddress address) {
    std::lock_guard<std::mutex> lock(mutex);
    this->myAddress = address;
}

ServerAddress ReplicaManager::getAddress() {
    std::lock_guard<std::mutex> lock(mutex); 
    return myAddress; 
}

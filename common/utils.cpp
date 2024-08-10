#include "utils.hpp"
#include <unistd.h>
#include <iostream>

u_int64_t getNumBlocks(u_int64_t fileSize, u_int64_t blockSize) {
    u_int64_t numBlocks = fileSize / blockSize;
    if (fileSize % blockSize != 0) {
        numBlocks++;
    }
    return numBlocks;
}

int buildFileIdFromPath(std::filesystem::path filepath, FileId *fileId) {
    std::ifstream file(filepath, std::ios::binary);

    if (!file) {
        return -1;
    }

    u_int64_t fileSize = std::filesystem::file_size(filepath);
    
    std::string filename = filepath.filename().string();

    fileId->totalBlocks = getNumBlocks(fileSize, MAX_PAYLOAD),
    fileId->fileSize = fileSize,
    fileId->filenameSize = static_cast<u_int8_t>(filename.length()),
    filename.copy(fileId->filename, MAX_FILENAME);
    return 0;
}

int openSocketTo(ServerAddress address) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    sockaddr_in addr = {
        .sin_family=AF_INET,
        .sin_port=address.port,
        .sin_addr={
            .s_addr=address.ip
        }
    };
    if (connect(sock, (sockaddr*) &addr, sizeof(addr)) == -1) {
        close(sock);
        return -1;
    }
    return sock;
}

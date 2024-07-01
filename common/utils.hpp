#pragma once

#include <sys/types.h>

#include <filesystem>
#include <iostream>
#include <fstream>

#include "Messages.hpp"

inline u_int64_t getNumBlocks(u_int64_t fileSize, u_int64_t blockSize) {
    u_int64_t numBlocks = fileSize / blockSize;
    if (fileSize % blockSize != 0) {
        numBlocks++;
    }
    return numBlocks;
}

inline int buildFileIdFromPath(std::filesystem::path filepath, FileId *fileId) {
    std::ifstream file(filepath, std::ios::binary);

    if (!file) {
        return -1;
    }
    
    file.seekg(std::ios::end);
    u_int64_t fileSize = file.tellg();
    file.seekg(std::ios::beg);
    
    std::string filename = filepath.filename().string();

    fileId->totalBlocks = getNumBlocks(fileSize, MAX_PAYLOAD),
    fileId->fileSize = fileSize,
    fileId->filenameSize = static_cast<u_int8_t>(filename.length() + 1),
    filename.copy(fileId->filename, MAX_FILENAME);
    return 0;
}
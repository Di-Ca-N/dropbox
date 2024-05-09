#pragma once

#include <sys/types.h>
#include <filesystem>
#include "Messages.h"

u_int64_t getNumBlocks(u_int64_t fileSize, u_int64_t blockSize) {
    u_int64_t numBlocks = fileSize / blockSize;
    if (fileSize % blockSize != 0) {
        numBlocks++;
    }
    return numBlocks;
}

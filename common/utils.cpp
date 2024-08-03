#include "utils.hpp"

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

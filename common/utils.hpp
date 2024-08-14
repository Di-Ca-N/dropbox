#include <sys/types.h>
#include <filesystem>
#include <fstream>
#include "Messages.hpp"

u_int64_t getNumBlocks(u_int64_t fileSize, u_int64_t blockSize);
int buildFileIdFromPath(std::filesystem::path filepath, FileId *fileId);
int openSocketTo(ServerAddress address);

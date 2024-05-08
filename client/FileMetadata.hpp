#ifndef FILE_METADATA_H
#define FILE_METADATA_H

#include <string>
#include <ctime>

struct FileMetadata {
    std::string filepath;
    time_t mtime;
    time_t atime;
    time_t ctime;
};

#endif

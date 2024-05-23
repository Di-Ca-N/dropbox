#include <iostream>

#include "FileMetadata.hpp"

void FileMetadata::print() {
    std::cout << filepath << std::endl;
    std::cout << "mtime: " << std::ctime(&mtime);
    std::cout << "atime: " << std::ctime(&atime);
    std::cout << "ctime: " << std::ctime(&ctime);
    std::cout << std::endl;
}

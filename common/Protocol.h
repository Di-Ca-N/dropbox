#include <filesystem>
#include "Messages.h"

class Protocol {
    private:
        int sock_fd;
    
    public:
        Protocol(int sock_fd);
        bool sendFile(std::filesystem::path filePath);
        std::string receiveFile(std::filesystem::path saveDir);    
};
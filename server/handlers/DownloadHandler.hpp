#include "Handler.hpp"
#include <string>

class DownloadHandler : public Handler {
    private:
        std::string username;
        int clientSocket;
    public:
        DownloadHandler(std::string username, int clientSocket);
        void run();
};


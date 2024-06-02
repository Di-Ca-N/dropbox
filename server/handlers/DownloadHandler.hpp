#include <string>

#include "ServerSocket.hpp"
#include "Handler.hpp"

class DownloadHandler : public Handler {
    private:
        std::string username;
        ServerSocket clientSocket;

    public:
        DownloadHandler(std::string username, ServerSocket clientSocket);
        void run();
};


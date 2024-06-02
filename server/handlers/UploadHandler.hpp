#include <string>

#include "ServerSocket.hpp"
#include "Handler.hpp"

class UploadHandler : public Handler {
    private:
        std::string username;
        ServerSocket clientSocket;

    public:
        UploadHandler(std::string username, ServerSocket clientSocket);
        void run();
};


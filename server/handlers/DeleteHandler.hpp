#include <string>

#include "ServerSocket.hpp"
#include "Handler.hpp"

class DeleteHandler : public Handler {
    private:
        std::string username;
        ServerSocket clientSocket;

    public:
        DeleteHandler(std::string username, ServerSocket clientSocket);
        void run();
};

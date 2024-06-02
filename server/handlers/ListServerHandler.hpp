#include <string>

#include "ServerSocket.hpp"
#include "Handler.hpp"

class ListServerHandler : public Handler {
    private:
        std::string username;
        ServerSocket clientSocket;
    public:
        ListServerHandler(std::string username, ServerSocket clientSocket);
        void run();
};

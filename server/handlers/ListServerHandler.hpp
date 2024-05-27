#include "Handler.hpp"
#include <string>

class ListServerHandler : public Handler {
    private:
        std::string username;
        int clientSocket;
    public:
        ListServerHandler(std::string username, int clientSocket);
        void run();
};

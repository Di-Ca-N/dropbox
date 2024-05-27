#include "Handler.hpp"
#include <string>

class DeleteHandler : public Handler {
    private:
        std::string username;
        int clientSocket;

    public:
        DeleteHandler(std::string username, int clientSocket);
        void run();
};

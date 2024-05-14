#include "Handler.hpp"
#include <string>

class UploadHandler : public Handler {
    private:
        std::string username;
        int clientSocket;

    public:
        UploadHandler(std::string username, int clientSocket);
        void run();
};


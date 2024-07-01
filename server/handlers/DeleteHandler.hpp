#include "Handler.hpp"
#include "../DeviceManager.hpp"
#include <string>

class DeleteHandler : public Handler {
    private:
        std::string username;
        int clientSocket;
        DeviceManager *deviceManager;

    public:
        DeleteHandler(std::string username, int clientSocket, DeviceManager *deviceManager);
        void run();
};

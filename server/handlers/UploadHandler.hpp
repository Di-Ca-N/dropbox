#include "Handler.hpp"
#include "../DeviceManager.hpp"
#include <string>

class UploadHandler : public Handler {
    private:
        std::string username;
        int clientSocket;
        DeviceManager *deviceManager;

    public:
        UploadHandler(std::string username, int clientSocket, DeviceManager *deviceManager);
        void run();
};


#include "Handler.cpp"
#include "../DeviceManager.hpp"
#include <string>

class RegisterDeviceHandler : public Handler {
    private:
        int clientSocket;
        std::string username;
        DeviceManager *deviceManager;

    public:
        RegisterDeviceHandler(std::string username, int clientSocket, DeviceManager *deviceManager);
        void run();
};
#include "Handler.hpp"
#include <string>
#include <filesystem>
#include "../DeviceManager.hpp"

class SyncClientToServerHandler : public Handler {
    private:
        int clientSocket;
        std::string username;
        int deviceId;
        DeviceManager *deviceManager;
        std::filesystem::path baseDir;

        void handleFileChange();
        void handleFileDelete();

    public:
        SyncClientToServerHandler(std::string username, int clientSocket, int deviceId, DeviceManager *DeviceManager);
        void run();
};
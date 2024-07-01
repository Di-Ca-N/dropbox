#include "Handler.hpp"
#include "../DeviceManager.hpp"
#include <string>

class SyncServerToClientHandler : public Handler {
    private:
        std::string username;
        int clientSocket;
        DeviceManager *deviceManager;
        std::filesystem::path baseDir;

        void handleFileModify(std::string filename);
        void handleFileDelete();
        void handleFileMove();
    public:
        SyncServerToClientHandler(std::string username, int clientSocket, DeviceManager *deviceManager);
        void run();
};


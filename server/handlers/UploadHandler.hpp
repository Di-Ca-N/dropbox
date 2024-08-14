#include "Handler.hpp"
#include "../DeviceManager.hpp"
#include "../ReplicaManager.hpp"
#include <string>

class UploadHandler : public Handler {
    private:
        std::string username;
        int clientSocket;
        DeviceManager *deviceManager;
        ReplicaManager *replicaManager;

    public:
        UploadHandler(std::string username, int clientSocket, DeviceManager *deviceManager, ReplicaManager *replicaManager);
        void run();
};


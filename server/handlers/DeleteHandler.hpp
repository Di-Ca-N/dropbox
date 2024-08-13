#include "Handler.hpp"
#include "../DeviceManager.hpp"
#include "../ReplicaManager.hpp"
#include <string>

class DeleteHandler : public Handler {
    private:
        std::string username;
        int clientSocket;
        DeviceManager *deviceManager;
        ReplicaManager *replicaManager;

    public:
        DeleteHandler(std::string username, int clientSocket, DeviceManager *deviceManager, ReplicaManager *replicaManager);
        void run();
};

#include "RegisterDeviceHandler.hpp"
#include "Messages.hpp"

RegisterDeviceHandler(std::string username, int clientSocket, DeviceManager *deviceManager) {
    this->username = username;
    this->clientSocket = clientSocket;
    this->deviceManager = deviceManager;
}

void RegisterDeviceHandler::run() {
    try {
        sendOk(clientSocket);

        Device device = deviceManager->registerDevice();
        sendDeviceId(clientSocket, device.id);

        waitConfirmation(clientSocket);
    } catch (UnexpectedMsgType) {
        sendError(clientSocket, "Unexpected message");
    } catch (ErrorReply e) {
        std::cout << "Error:" << e.what() << "\n";
    }
}
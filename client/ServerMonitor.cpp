#include <memory>
#include <unistd.h>
#include <iostream>

#include "ServerMonitor.hpp"
#include "ClientState.hpp"
#include "Connection.hpp"
#include "Messages.hpp"
#include "ClientConfig.hpp"

ServerMonitor::ServerMonitor(
        std::shared_ptr<ClientState> clientState,
        std::shared_ptr<Connection> connection,
        std::shared_ptr<EventHistory> history) {

    this->clientState = clientState;
    this->connection = connection;
    this->history = history;
}

void ServerMonitor::run() {
    std::optional<FileOperation> operation;

    while (clientState->get() == AppState::STATE_ACTIVE) {
        operation = connection->syncRead();

        if (operation.has_value()) {
            history->pushEvent(operation.value());             
            applyTempIfContentUpdate(operation.value());
            operation.reset();
        }
    }
}

void ServerMonitor::applyTempIfContentUpdate(FileOperation &operation) {
    std::string targetString;
    std::ifstream tempStream;
    std::ofstream targetStream;
    std::filesystem::path targetPath, tempPath;

    if (operation.type == FileOpType::FILE_MODIFY) {
        targetString = std::string(operation.filename, operation.filenameSize); 
        targetPath = std::filesystem::path(SYNC_DIR) / targetString;

        tempPath = targetPath;
        tempPath += TEMP_FILE_EXT;

        tempStream.open(tempPath, std::ios::binary);
        targetStream.open(targetPath, std::ios::binary);

        if (tempStream.is_open() && targetStream.is_open()) {
            targetStream << tempStream.rdbuf();
        }

        tempStream.close();
        targetStream.close();

        std::remove(tempPath.c_str());
    }
}


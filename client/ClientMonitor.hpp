#ifndef CLIENT_MONITOR_H
#define CLIENT_MONITOR_H

#include <memory>
#include <string>
#include <stdexcept>
#include <sys/inotify.h>
#include <queue>

#include "ClientState.hpp"
#include "Connection.hpp"
#include "EventHistory.hpp"

class ClientMonitor {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;
    std::shared_ptr<EventHistory> history;

    std::queue<FileOperation> queue = std::queue<FileOperation>();

    void watchEvents(int inotifyFd, int bitMask, std::string syncDir);
    void setNonBlocking(int inotifyFd);
    void processEventBuffer(unsigned char buffer[], int bytesRead);
    bool fileIsTemporary(char* name, int len);
    void removeTrailingZeros(std::string &str);
    void sendOperationIfNotDuplicated(FileOperation &operation);
    FileOperation makeFileOperation(FileOpType opType, char *fileName, size_t nameLength);
    int startEventTracking();
public:
    ClientMonitor(
            std::shared_ptr<ClientState> clientState,
            std::shared_ptr<Connection> connection,
            std::shared_ptr<EventHistory> history);
    void run(std::string sync_dir);
};

class TrackingErrorException : public std::runtime_error {
public:
    TrackingErrorException(const std::string& message)
        : std::runtime_error(message) {}
};

#endif

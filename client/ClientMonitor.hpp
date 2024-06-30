#ifndef CLIENT_MONITOR_H
#define CLIENT_MONITOR_H

#include <memory>
#include <string>
#include <stdexcept>
#include <sys/inotify.h>

#include "ClientState.hpp"
#include "Connection.hpp"
#include "EventHistory.hpp"

class ClientMonitor {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;
    std::shared_ptr<EventHistory> history;

    void watchEvents(int inotifyFd, int bitMask, std::string syncDir);
    void setNonBlocking(int inotifyFd);
    void processEventBuffer(unsigned char buffer[], int bytesRead);
    void sendOperationIfNotDuplicated(FileOpType opType, inotify_event *event);
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

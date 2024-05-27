#ifndef CLIENT_MONITOR_H
#define CLIENT_MONITOR_H

#include <memory>
#include <string>
#include <stdexcept>

#include "ClientState.hpp"
#include "Connection.hpp"

class ClientMonitor {
    std::shared_ptr<ClientState> clientState;
    std::shared_ptr<Connection> connection;

    void watchEvents(int inotifyFd, int bitMask, std::string syncDir);
    void setNonBlocking(int inotifyFd);
    void processEventBuffer(unsigned char buffer[], int bytesRead);
    int startEventTracking();
public:
    ClientMonitor(std::shared_ptr<ClientState> clientState,
            std::shared_ptr<Connection> connection);
    void run(std::string sync_dir);
};

class TrackingErrorException : public std::runtime_error {
public:
    TrackingErrorException(const std::string& message)
        : std::runtime_error(message) {}
};

#endif

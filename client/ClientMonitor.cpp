#include <memory>
#include <fcntl.h>
#include <sys/unistd.h>
#include <sys/inotify.h>

#include "ClientMonitor.hpp"
#include "ClientState.hpp"
#include "Messages.hpp"
#include "Connection.hpp"

#define MAX_EVENT_SIZE (sizeof(struct inotify_event) + 256)
#define MAX_EVENTS 10
#define EVENT_BUF_LEN (MAX_EVENT_SIZE * MAX_EVENTS)

ClientMonitor::ClientMonitor(std::shared_ptr<ClientState> clientState,
        std::shared_ptr<Connection> connection) {
    this->clientState = clientState;
    this->connection = connection;
}

void ClientMonitor::run(std::string sync_dir) {
    int inotifyFd, bytesRead;
    unsigned char buffer[EVENT_BUF_LEN];

    try {
        inotifyFd = startEventTracking();
        watchEvents(
                inotifyFd,
                IN_CLOSE_WRITE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO,
                sync_dir
        );
        setNonBlocking(inotifyFd);
    } catch (TrackingErrorException &e) {
       clientState->setUntrackedIfNotClosing(); 
    }

    while (clientState->get() == AppState::STATE_ACTIVE) {
        bytesRead = read(inotifyFd, buffer, EVENT_BUF_LEN); 
        if (bytesRead > 0) {
            processEventBuffer(buffer, bytesRead);
        }
    }

    close(inotifyFd);
}

void ClientMonitor::watchEvents(int inotifyFd, int bitMask, std::string syncDir) {
    int watchFd = inotify_add_watch(inotifyFd, syncDir.c_str(), bitMask);
    if (watchFd == -1)
        throw TrackingErrorException("Couldn't add inotify watch");
}

void ClientMonitor::setNonBlocking(int inotifyFd) {
    int flags = fcntl(inotifyFd, F_GETFL, 0);
    fcntl(inotifyFd, F_SETFL, flags | O_NONBLOCK);
}

void ClientMonitor::processEventBuffer(unsigned char buffer[], int bytesRead) {
    unsigned char *eventPtr = buffer;
    while (eventPtr < buffer + bytesRead) {
        inotify_event *event = (inotify_event*) eventPtr;
        if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO)
            connection->syncWrite(FileOpType::FILE_MODIFY, event->name);
        if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
            connection->syncWrite(FileOpType::FILE_DELETE, event->name);
        if (event->mask & IN_DELETE_SELF)
            clientState->setUntrackedIfNotClosing();
        eventPtr += sizeof(inotify_event) + event->len;
    }
}

int ClientMonitor::startEventTracking() {
    int inotifyFd = inotify_init();
    if (inotifyFd < 0)
        throw TrackingErrorException("Couldn't start inotify");
    return inotifyFd;
}


#include <memory>
#include <iostream>
#include <fcntl.h>
#include <sys/unistd.h>
#include <sys/inotify.h>

#include "ClientMonitor.hpp"
#include "ClientState.hpp"
#include "FileOp.hpp"
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
    int inotifyFd, watchFd, flags, bytesRead;
    unsigned char buffer[EVENT_BUF_LEN];

    inotifyFd = inotify_init();

    if (inotifyFd < 0) {
        std::cerr << "Couldn't open inotify file" << std::endl;
        return;
    }

    watchFd = inotify_add_watch(inotifyFd, sync_dir.c_str(),
            IN_CLOSE_WRITE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO);

    if (watchFd == -1) {
        std::cerr << "Couldn't add inotify watch" << std::endl;
        return;
    }

    flags = fcntl(inotifyFd, F_GETFL, 0);
    fcntl(inotifyFd, F_SETFL, flags | O_NONBLOCK);

    while (clientState->get() == AppState::STATE_ACTIVE) {
        bytesRead = read(inotifyFd, buffer, EVENT_BUF_LEN); 

        if (bytesRead > 0) {
            unsigned char *eventPtr = buffer;
            while (eventPtr < buffer + bytesRead) {
                inotify_event *event = (inotify_event*) eventPtr;
                if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO)
                    connection->syncWrite(FileOp::OP_CHANGE, event->name, event->name);
                if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
                    connection->syncWrite(FileOp::OP_DELETE, event->name, event->name);
                if (event->mask & IN_DELETE_SELF)
                    clientState->set(AppState::STATE_UNTRACKED);
                eventPtr += sizeof(inotify_event) + event->len;
            }
        }
    }
}


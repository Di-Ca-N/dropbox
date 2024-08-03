#include <memory>
#include <fcntl.h>
#include <sys/unistd.h>
#include <cstring>

#include "ClientMonitor.hpp"
#include "ClientState.hpp"
#include "Messages.hpp"
#include "ClientConfig.hpp"
#include "Connection.hpp"

#define MAX_EVENT_SIZE (sizeof(struct inotify_event) + 256)
#define MAX_EVENTS 10
#define EVENT_BUF_LEN (MAX_EVENT_SIZE * MAX_EVENTS)

ClientMonitor::ClientMonitor(
        std::shared_ptr<ClientState> clientState,
        std::shared_ptr<Connection> connection,
        std::shared_ptr<EventHistory> history) {
    this->clientState = clientState;
    this->connection = connection;
    this->history = history;
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

    try {
        while (clientState->get() == AppState::STATE_ACTIVE) {
            bytesRead = read(inotifyFd, buffer, EVENT_BUF_LEN); 
            if (bytesRead > 0) {
                processEventBuffer(buffer, bytesRead);
            }
        }
    } catch (BrokenPipe) {
        close(inotifyFd);
        return;
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
    unsigned char *eventPtr;
    inotify_event *event;

    eventPtr = buffer;

    while (eventPtr < buffer + bytesRead) {
        event = (inotify_event*) eventPtr;

        if (!fileIsTemporary(event->name, event->len)) {
            if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO) {
                //std::cout << "MODIFY\n";
                sendOperationIfNotDuplicated(FileOpType::FILE_MODIFY, event);
            }

            if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM){
                //std::cout << "DELETE\n";
                sendOperationIfNotDuplicated(FileOpType::FILE_DELETE, event);
            }
        }

        if (event->mask & IN_DELETE_SELF)
            clientState->setUntrackedIfNotClosing();

        eventPtr += sizeof(inotify_event) + event->len;
    }
}

bool ClientMonitor::fileIsTemporary(char* name, int len) {
    std::string fileName = std::string(name, len);
    auto filePath = std::filesystem::path(fileName);

    auto tempExtension = std::string(TEMP_FILE_EXT);
    std::string fileExtension = filePath.extension().string();

    removeTrailingZeros(fileExtension);

    return (fileExtension == tempExtension);
}

void ClientMonitor::removeTrailingZeros(std::string &str) {
    auto it = str.begin(); 
    for (; *it != '\0'; it++);

    if (it != str.end()) {
        str.erase(it, str.end());
    }
}

void ClientMonitor::sendOperationIfNotDuplicated(
        FileOpType opType,
        inotify_event *event) {
    auto operation = makeFileOperation(opType, event->name, event->len);
    std::string eventType = operation.type == FileOpType::FILE_MODIFY ? "Change" : "Other";
    //std::cout << "Registered event " << eventType << " on file " << std::string(operation.filename, operation.filenameSize).size() << "\n";

    if (!history->popEvent(operation))
        connection->syncWrite(opType, event->name);
}

FileOperation ClientMonitor::makeFileOperation(
        FileOpType opType,
        char *fileName,
        size_t nameLength) {
    size_t copyLength;
    FileOperation operation;

    operation.type = opType;
    std::string filename(fileName);
    filename.copy(operation.filename, MAX_FILENAME);
    operation.filenameSize = filename.size();

    return operation;
}

int ClientMonitor::startEventTracking() {
    int inotifyFd = inotify_init();
    if (inotifyFd < 0)
        throw TrackingErrorException("Couldn't start inotify");
    return inotifyFd;
}


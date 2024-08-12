#include <memory>
#include <iostream>
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

    while (clientState->get() == AppState::STATE_ACTIVE) {
        bytesRead = read(inotifyFd, buffer, EVENT_BUF_LEN); 
        processEventBuffer(buffer, bytesRead);
        try {
            sendOperationIfNotDuplicated(queue.front());
            queue.pop();
        } catch (ErrorReply e) {
            std::cout << "Error: " << e.what() << "\n";
        } catch (UnexpectedMsgType) {
            std::cout << "Unexpected response\n";
        } catch (BrokenPipe) {
            // Não faz nada
        } catch (ServerConnectionError) {
            // Não faz nada
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
    unsigned char *eventPtr;
    inotify_event *event;

    eventPtr = buffer;

    while (eventPtr < buffer + bytesRead) {
        event = (inotify_event*) eventPtr;

        if (!fileIsTemporary(event->name, event->len)) {
            if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO) {
                auto operation = makeFileOperation(
                        FileOpType::FILE_MODIFY,
                        event->name,
                        event->len
                );
                queue.push(operation);
            }

            if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM){
                auto operation = makeFileOperation(
                        FileOpType::FILE_DELETE,
                        event->name,
                        event->len
                );
                queue.push(operation);
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

void ClientMonitor::sendOperationIfNotDuplicated(FileOperation &operation) {
    if (!history->popEvent(operation)) {
        auto lastCharIndex = operation.filename + operation.filenameSize;
        std::filesystem::path filePath(operation.filename, lastCharIndex);
        connection->syncWrite(operation.type, filePath);
    }
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


#include <cstring>

#include "EventHistory.hpp"

EventHistory::EventHistory() {
    history = std::list<FileOperation>();
}

void EventHistory::pushEvent(FileOperation event) {
    std::lock_guard<std::mutex> lock(mtx);
    history.push_back(event);
}

bool EventHistory::popEvent(FileOperation event) {
    std::list<FileOperation>::iterator iter;

    std::lock_guard<std::mutex> lock(mtx);

    for (iter = history.begin(); iter != history.end(); iter++) {
        if (typesAreEqual(*iter, event)
                && filenamesAreEqual(*iter, event)) {
            break;
        }
    }

    if (iter != history.end()) {
        history.erase(iter);
        return true;
    }

    return false;
}

bool EventHistory::typesAreEqual(FileOperation &eventA, FileOperation &eventB) {
    return (eventA.type == eventB.type);
}

bool EventHistory::filenamesAreEqual(
        FileOperation &eventA,
        FileOperation &eventB) { 
    if (eventA.filenameSize != eventB.filenameSize)
        return false;

    if (strncmp(eventA.filename, eventB.filename, eventA.filenameSize) == 0)
        return true;

    return false;
}


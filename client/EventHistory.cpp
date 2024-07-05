#include <cstring>
#include <iostream>
#include "EventHistory.hpp"

EventHistory::EventHistory() {
    history = std::list<FileOperation>();
}

void EventHistory::pushEvent(FileOperation event) {
    //std::cout << "Pushing event on " << std::string(event.filename, event.filenameSize).size() << "\n";
    std::lock_guard<std::mutex> lock(mtx);
    history.push_back(event);
}

bool EventHistory::popEvent(FileOperation event) {
    std::list<FileOperation>::iterator iter;

    std::lock_guard<std::mutex> lock(mtx);

    for (iter = history.begin(); iter != history.end(); iter++) {
        std::string eventType = event.type == FileOpType::FILE_MODIFY ? "Change" : "Other";
        //std::cout << "History contains event of type " << eventType << " for file " << std::string(event.filename, event.filenameSize) << "\n";
        if (typesAreEqual(*iter, event) && filenamesAreEqual(*iter, event)) {
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
    //std::cout << "Type comparison: " << (eventA.type == eventB.type) << "\n";
    return (eventA.type == eventB.type);
}

bool EventHistory::filenamesAreEqual(
        FileOperation &eventA,
        FileOperation &eventB) {
    std::string filenameA(eventA.filename, eventA.filenameSize);
    std::string filenameB(eventB.filename, eventB.filenameSize);
    //std::cout << "NameA: " << filenameA << filenameA.size() << " NameB: " << filenameB << filenameB.size() << "\n";
    //std::cout << "Name comparison: " << (filenameA == filenameB) << "\n";

    return filenameA.compare(filenameB) == 0;
}


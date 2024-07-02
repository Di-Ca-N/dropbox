#ifndef EVENT_HISTORY_H
#define EVENT_HISTORY_H

#include <mutex>
#include <list>

#include "Messages.hpp"

class EventHistory {
private:
    std::mutex mtx;
    std::list<FileOperation> history;

    bool typesAreEqual(FileOperation &eventA, FileOperation &eventB);
    bool filenamesAreEqual(FileOperation &eventA, FileOperation &eventB); 

public:
    EventHistory();
    void pushEvent(FileOperation event); 
    bool popEvent(FileOperation event);
};

#endif

#pragma once

#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <semaphore.h>
#include "Messages.hpp"


class SyncQueue {
    private:
        std::queue<FileOperation> opQueue;
        std::mutex mutex;
        std::condition_variable hasOperation;
        sem_t sem;

    public:
        SyncQueue();
        // Pushes a FileOperation into the queue
        void push(FileOperation fp);
        // Tries to get a FileOperation from the Queue. Blocks until an operation is available
        FileOperation get();
};

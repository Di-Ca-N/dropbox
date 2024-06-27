#include <string>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include "Messages.hpp"


class SyncQueue {
    private:
        std::queue<FileOperation> opQueue;
        sem_t sem;

    public:
        SyncQueue();
        void push(FileOperation fp);
        FileOperation get();
};

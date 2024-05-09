#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <semaphore.h>

struct FileOperation {
    int opType;
    std::string file;
};

class SyncQueue {
    private:
        std::queue<FileOperation> opQueue;
        sem_t sem;

    public:
        SyncQueue();
        void push(FileOperation fp);
        FileOperation get();
};
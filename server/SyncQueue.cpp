#include "SyncQueue.hpp"

SyncQueue::SyncQueue() {}

void SyncQueue::push(FileOperation fp) {
    std::lock_guard<std::mutex> lock(mutex);
    opQueue.push(fp);
    hasOperation.notify_one();
}

FileOperation SyncQueue::get() {
    std::unique_lock<std::mutex> lock(mutex);
    while (opQueue.empty()) {
        hasOperation.wait(lock);
    }
    FileOperation op = opQueue.front();
    opQueue.pop();
    return op;
}

std::optional<FileOperation> SyncQueue::get(int timeoutMs) {
    std::unique_lock<std::mutex> lock(mutex);

    bool operationAvailable = hasOperation.wait_for(lock, std::chrono::milliseconds(timeoutMs), [&]{return !opQueue.empty();});

    if (operationAvailable) {
        FileOperation op = opQueue.front();
        opQueue.pop();
        return op;
    } else {
        return std::nullopt;
    }
}

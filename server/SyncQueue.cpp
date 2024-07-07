#include "SyncQueue.hpp"

SyncQueue::SyncQueue() {}

void SyncQueue::push(FileOperation fp) {
    std::lock_guard<std::mutex> lock(mutex);
    opQueue.push(fp);
    hasOperation.notify_one();
}

FileOperation SyncQueue::get() {
    std::unique_lock<std::mutex> lock(mutex);
    while (opQueue.empty()) hasOperation.wait(lock);
    FileOperation op = opQueue.front();
    opQueue.pop();
    return op;
}

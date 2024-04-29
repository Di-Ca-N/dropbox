#include "SyncQueue.h"

SyncQueue::SyncQueue() {
    sem_init(&sem, 0, 0);
}

void SyncQueue::push(FileOperation fp) {
    opQueue.push(fp);
    sem_post(&sem);
}

FileOperation SyncQueue::get() {
    sem_wait(&sem);
    FileOperation op = opQueue.front();
    opQueue.pop();
    return op;
}

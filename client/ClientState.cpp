#include "ClientState.hpp"

ClientState::ClientState(AppState state) {
    sem_init(&mutex, 0, 1);
    sem_init(&rw, 0, 1);
    this->readCount = 0;
    this->state = state;
}

ClientState::~ClientState() {
    sem_destroy(&mutex);
    sem_destroy(&rw);
}

AppState ClientState::get() {
    AppState state;

    sem_wait(&mutex);
    readCount++;
    if (readCount == 1)
        sem_wait(&rw);
    sem_post(&mutex);

    state = this->state;

    sem_wait(&mutex);
    readCount--;
    if (readCount == 0)
        sem_post(&rw);
    sem_post(&mutex);

    return state;
}

void ClientState::set(AppState state) {
    sem_wait(&rw);
    this->state = state;
    sem_post(&rw);
}

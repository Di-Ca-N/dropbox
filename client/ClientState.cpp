#include "ClientState.hpp"

ClientState::ClientState(AppState state) {
    sem_init(&mutex, 0, 1);
    this->state = state;
}

ClientState::~ClientState() {
    sem_destroy(&mutex);
}

AppState ClientState::get() {
    return state;   // at-most-once
}

void ClientState::setActiveIfNotClosing() {
    sem_wait(&mutex);
    if (state != AppState::STATE_CLOSING)
        state = AppState::STATE_ACTIVE;
    sem_post(&mutex);
}

void ClientState::setUntrackedIfNotClosing() {
    sem_wait(&mutex);
    if (state != AppState::STATE_CLOSING)
        state = AppState::STATE_UNTRACKED;
    sem_post(&mutex);
}

void ClientState::setClosing() {
    sem_wait(&mutex);
    state = AppState::STATE_CLOSING;
    sem_post(&mutex);
}

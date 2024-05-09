#ifndef CLIENT_STATE_H
#define CLIENT_STATE_H

#include <semaphore.h>

enum class AppState {
    STATE_ACTIVE,
    STATE_UNTRACKED,
    STATE_CLOSING
};

class ClientState {
    sem_t mutex;
    sem_t rw;
    int readCount;
    AppState state;
public:
    ClientState(AppState state);
    ~ClientState();
    AppState get();
    void set(AppState state);
};

#endif

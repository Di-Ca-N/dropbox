#ifndef THREAD_OWNER_H
#define THREAD_OWNER_H

class ThreadOwner {
    public:
        virtual void restartServerThread() = 0;
        virtual void restartClientThread() = 0;
};

#endif

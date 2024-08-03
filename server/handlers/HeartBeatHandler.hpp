#include "Handler.hpp"

class HeartBeatHandler : public Handler {
    private:
        int remoteSocket;
    public:
        HeartBeatHandler(int remoteSocket);
        void run();
};
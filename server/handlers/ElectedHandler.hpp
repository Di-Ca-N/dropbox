#include "Handler.hpp"
#include "Messages.hpp"
#include "../ElectionManager.hpp"


class ElectedHandler : public Handler {
    private:
        int replicaSocket;
        ServerAddress nextServerAddr;
        int id;
        ElectionManager *manager;
    public:
        ElectedHandler(int replicaSocket, int myId, ServerAddress nextServerAddr, ElectionManager *manager);
        void run();
};

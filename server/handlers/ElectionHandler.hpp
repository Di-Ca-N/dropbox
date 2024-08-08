#include "Handler.hpp"
#include "Messages.hpp"
#include "../ElectionManager.hpp"


class ElectionHandler : Handler {
    private:
        int replicaSocket;
        ServerAddress myAddress;
        ServerAddress nextAddress;
        int myId;
        ElectionManager *manager;
    public:
        ElectionHandler(int replicaSocket, ServerAddress myAddress, int myId, ServerAddress nextAddress, ElectionManager *manager);
        void run();
};

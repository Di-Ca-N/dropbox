#include "Handler.hpp"
#include "Messages.hpp"
#include "../ElectionManager.hpp"
#include "../ReplicaManager.hpp"


class ElectionHandler : Handler {
    private:
        int replicaSocket;
        int myId;
        ElectionManager *electionManager;
        ReplicaManager *replicaManager;
    public:
        ElectionHandler(int replicaSocket, int myId, ElectionManager *electionManager, ReplicaManager *replicaManager);
        void run();
};

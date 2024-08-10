#include "Handler.hpp"
#include "Messages.hpp"
#include "../ElectionManager.hpp"
#include "../ReplicaManager.hpp"


class ElectedHandler : public Handler {
    private:
        int replicaSocket;
        ServerAddress myAddr;
        int id;
        ElectionManager *electionManager;
        ReplicaManager *replicaManager;
    public:
        ElectedHandler(int replicaSocket, int myId, ServerAddress myAddr, ReplicaManager *replicaManager, ElectionManager *manager);
        void run();
};

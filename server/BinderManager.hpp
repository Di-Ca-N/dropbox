#pragma once

#include "Messages.hpp"

class BinderManager {
    private:
        ServerAddress binderAddress; 

        int createSocket(in_addr_t ip, in_port_t port);

    public:
        BinderManager(ServerAddress binderAddress);
        void notifyBinder(ServerAddress primaryAddress);
};

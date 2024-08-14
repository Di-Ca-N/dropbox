#pragma once

#include "Messages.hpp"

class BinderHandler {
    private:
        int binderSock;

    public:
        BinderHandler(int binderSock);
        void sendAddress(ServerAddress primaryAddress);
};

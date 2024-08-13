#pragma once

#include "semaphore.h"
#include "BinderManager.hpp"
#include <mutex>
#include <condition_variable>
#include "Messages.hpp"

class ElectionManager {
    private:
        int leaderId;
        ServerAddress leaderAddr;
        bool candidate = false;
        BinderManager *binderManager;
        std::mutex mutex;
        std::condition_variable electionRunning;
    public:
        ElectionManager(BinderManager *binderManager, int myId, int leaderId, ServerAddress leaderAddress);
        void setLeader(int myId, int leaderId, ServerAddress leaderAddress);
        int getLeader();
        ServerAddress getLeaderAddress();
        void finishElection();
        void waitElectionEnd(); 
        bool isParticipating();
        void markParticipation();
};

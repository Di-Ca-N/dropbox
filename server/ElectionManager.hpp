#pragma once

#include "semaphore.h"
#include <mutex>
#include <condition_variable>
#include "Messages.hpp"

class ElectionManager {
    private:
        int leaderId;
        bool candidate = false;
        std::mutex mutex;
        std::condition_variable electionRunning;
    public:
        ElectionManager(int leaderId, ServerAddress leaderAddress);
        void setLeader(int leaderId, ServerAddress leaderAddress);
        int getLeader();
        void finishElection();
        void waitElectionEnd(); 
        bool isParticipating();
        void markParticipation();
};

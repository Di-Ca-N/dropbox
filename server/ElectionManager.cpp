#include "ElectionManager.hpp"

#include "Messages.hpp"
#include <iostream>

ElectionManager::ElectionManager(BinderManager *binderManager, int myId, int leaderId, ServerAddress leaderAddress) {
    this->binderManager = binderManager;
    setLeader(myId, leaderId, leaderAddress);
}

int ElectionManager::getLeader() {
    return this->leaderId;
}

ServerAddress ElectionManager::getLeaderAddress() {
    return this->leaderAddr;
}

void ElectionManager::setLeader(int myId, int leaderId, ServerAddress leaderAddress) {
    this->leaderId = leaderId;
    this->leaderAddr = leaderAddress;

    if (leaderId == myId) {
        binderManager->notifyBinder(leaderAddress); 
    }
}

void ElectionManager::finishElection() {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "Election ended\n";
    candidate = false;
    electionRunning.notify_one();
}

void ElectionManager::waitElectionEnd() {
    std::unique_lock<std::mutex> lock(mutex);
    electionRunning.wait(lock);
}

bool ElectionManager::isParticipating() {
    return this->candidate; 
}

void ElectionManager::markParticipation() {
    this->candidate = true;
}

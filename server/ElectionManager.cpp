#include "ElectionManager.hpp"

#include "Messages.hpp"
#include <iostream>

ElectionManager::ElectionManager(int leaderId, ServerAddress leaderAddress) {
    setLeader(leaderId, leaderAddress);
}

int ElectionManager::getLeader() {
    return this->leaderId;
}

ServerAddress ElectionManager::getLeaderAddress() {
    return this->leaderAddr;
}

void ElectionManager::setLeader(int leaderId, ServerAddress leaderAddress) {
    this->leaderId = leaderId;
    this->leaderAddr = leaderAddress;
    // ToDo: Notify nameserver about new primary
}

void ElectionManager::finishElection() {
    std::lock_guard<std::mutex> lock(mutex);
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

#include "ElectedHandler.hpp"
#include <iostream>
#include "Messages.hpp"
#include "utils.hpp"

ElectedHandler::ElectedHandler(int replicaSocket, int myId, ReplicaManager *replicaManager, ElectionManager *manager) {
    this->replicaSocket = replicaSocket;
    this->id = myId;
    this->replicaManager = replicaManager;
    this->electionManager = manager;
}

void ElectedHandler::run() {
    try {
        sendOk(replicaSocket);
        Ballot ballot = receiveBallot(replicaSocket);
        std::cout << "Elected leader with id " << ballot.id << std::endl;
        sendOk(replicaSocket);

        if (ballot.id == electionManager->getLeader()) return;

        electionManager->setLeader(id, ballot.id, ballot.address);

        if (ballot.id != this->id) {
            ServerAddress nextServerAddr = replicaManager->getNextReplica();
            int nextServer = openSocketTo(nextServerAddr);
            if (nextServer == -1) {
                std::cout << "Could not connect to next server on ring\n";
                return;
            }
            AuthData authData = {
                .type=AuthType::AUTH_REPLICA,
            };
            sendAuth(nextServer, authData);
            receiveAuth(nextServer);

            sendMessage(nextServer, MsgType::MSG_ELECTED, nullptr, 0);
            waitConfirmation(nextServer);

            sendBallot(nextServer, ballot);
            waitConfirmation(nextServer);

            electionManager->finishElection();
        }
        replicaManager->clearReplicas();
        electionManager->finishElection();
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType e) {
        std::cout << "Unexpected msg: " << e.what() << "\n";
    }
}

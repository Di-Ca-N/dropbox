#include "ElectionHandler.hpp"

#include <iostream>

#include "Messages.hpp"
#include "utils.hpp"

ElectionHandler::ElectionHandler(
        int replicaSocket, int myId, ElectionManager *electionManager, ReplicaManager *replicaManager
) {
    this->replicaSocket = replicaSocket;
    this->myId = myId;
    this->electionManager = electionManager;
    this->replicaManager = replicaManager;
}

void ElectionHandler::run() {
    try {
        sendOk(replicaSocket);

        Ballot ballot = receiveBallot(replicaSocket);
        sendOk(replicaSocket);

        // Notify next server about the election
        int nextServer = openSocketTo(replicaManager->getNextReplica());
        AuthData authData = {
            .type=AuthType::AUTH_REPLICA, 
            .replicaData = {
                .replicaAddr=replicaManager->getAddress(),
                .replicaId=myId
            }
        };
        sendAuth(nextServer, authData);
        receiveAuth(nextServer);

        if (this->myId == ballot.id) { // Ballot came back. I'm the new leader
            std::cout << "Sending elected\n";
            sendMessage(nextServer, MsgType::MSG_ELECTED, nullptr, 0);
            waitConfirmation(nextServer);
            sendBallot(nextServer, ballot);
            waitConfirmation(nextServer);
            // Set Leader
        } else if (this->myId > ballot.id) {
            std::cout << "Forwarding ballot\n";
            ballot.id = myId;
            ballot.address = replicaManager->getAddress();
            sendMessage(nextServer, MsgType::MSG_ELECTION, nullptr, 0);
            waitConfirmation(nextServer);
            sendBallot(nextServer, ballot);
            waitConfirmation(nextServer);
        } else {
            sendMessage(nextServer, MsgType::MSG_ELECTION, nullptr, 0);
            waitConfirmation(nextServer);
            sendBallot(nextServer, ballot);
            waitConfirmation(nextServer);
        }
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected message\n";
    } catch (ErrorReply e) {
        std::cout << "Error during file election: " << e.what() << "\n";
    }
}

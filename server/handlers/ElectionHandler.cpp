#include "ElectionHandler.hpp"

#include <iostream>

#include "Messages.hpp"
#include "utils.hpp"

ElectionHandler::ElectionHandler(
        int replicaSocket, ServerAddress myAddress, int myId, 
        ServerAddress nextAddress, ElectionManager *manager
) {
    this->replicaSocket = replicaSocket;
    this->myAddress = myAddress;
    this->myId = myId;
    this->nextAddress = nextAddress;
    this->manager = manager;
}

void ElectionHandler::run() {
    std::cout << "ElectionHandler\n";
    try {
        sendOk(replicaSocket);

        Ballot ballot = receiveBallot(replicaSocket);
        sendOk(replicaSocket);

        // Notify next server about the election
        int nextServer = openSocketTo(nextAddress);
        AuthData authData = {
            .type=AuthType::AUTH_REPLICA, 
            .replicaData = {
                .ipAddress=myAddress.ip,
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
            ballot.address = myAddress;
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

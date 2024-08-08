#include "ElectedHandler.hpp"
#include <iostream>
#include "Messages.hpp"
#include "utils.hpp"

ElectedHandler::ElectedHandler(int replicaSocket, int myId, ServerAddress nextServerAddr, ElectionManager *manager) {
    this->replicaSocket = replicaSocket;
    this->id = myId;
    this->nextServerAddr = nextServerAddr;
    this->manager = manager;
}

void ElectedHandler::run() {
    try {
        sendOk(replicaSocket);
        Ballot ballot = receiveBallot(replicaSocket);
        std::cout << "Elected leader with id " << ballot.id << std::endl;
        sendOk(replicaSocket);
        manager->setLeader(ballot.id, ballot.address);
        if (ballot.id == this->id) return;

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
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType e) {
        std::cout << "Unexpected msg: " << e.what() << "\n";
    }
}

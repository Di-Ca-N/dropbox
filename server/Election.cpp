#include <sys/socket.h>
#include <vector>
#include "Messages.hpp"
#include "utils.hpp"

void triggerElection(ServerAddress nextAddress, ServerAddress myAddress, ) {
    int nextServerSock = openSocketTo(nextAddress);

    sendBallot()
}
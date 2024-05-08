#include "Messages.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <map>

std::map<MsgType, std::string> msgTypeNames = {
    {MsgType::MSG_AUTH, "MSG_AUTH"},
    {MsgType::MSG_SYNC, "MSG_SYNC"},
    {MsgType::MSG_OK, "MSG_OK"},
    {MsgType::MSG_ERROR, "MSG_ERROR"},
    {MsgType::MSG_UPLOAD, "MSG_UPLOAD"},
    {MsgType::MSG_DOWNLOAD, "MSG_DOWNLOAD"},
    {MsgType::MSG_LIST_SERVER, "MSG_LIST_SERVER"},
    {MsgType::MSG_FILE_ID, "MSG_FILE_ID"},
    {MsgType::MSG_FILEPART, "MSG_FILEPART"},
};


int receiveMessage(int sock_fd, Message *msg) {
    char buffer[sizeof(Message)];
    int totalRead = 0;
    while (totalRead < sizeof(Message)) {
        int readBytes = recv(sock_fd, buffer + totalRead, sizeof(Message) - totalRead, 0);
        if (readBytes == 0)
            return ERROR_BROKEN_PIPE;
        totalRead += readBytes;
    };
    memcpy(msg, buffer, sizeof(Message));
    //std::cout << "Read bytes: " << readBytes << " is equal to message size?" << (sizeof(Message) == readBytes) << "\n";
    return 0;
}

int sendMessage(int sock_fd, MsgType type, void* msgPayload, unsigned int payloadLen) {
    if (payloadLen > MAX_PAYLOAD) {
        return ERROR_PAYLOAD_TOO_BIG;
    }
    Message msg = {
        .type=type,
        .len=payloadLen,
    };
    memcpy(msg.payload, msgPayload, payloadLen);

    // Using MSG_NOSIGNAL to avoid receiving SIGPIPE when the socket is closed
    if (send(sock_fd, &msg, sizeof(msg), MSG_NOSIGNAL) == -1)
        return ERROR_BROKEN_PIPE;
    return 0;
}

void printMsg(Message *msg) {
    std::cout << "=== MESSAGE BEGIN ===\n";
    std::cout << "TYPE    : " << msgTypeNames[msg->type] << "\n";
    std::cout << "LEN     : " << msg->len << "\n";
    std::cout << "---- PAYLOAD ----\n";

    switch (msg->type) {
        case MsgType::MSG_FILE_ID: {
            FileId *payload = (FileId*) msg->payload;

            std::cout << "TOTAL BLOCKS : " << payload->totalBlocks << "\n";
            std::cout << "FILE SIZE    : " << payload->fileSize << "\n";
            std::cout << "FILENAME LEN : " << unsigned(payload->filenameSize) << "\n";
            std::cout << "FILENAME     : " << std::string(payload->filename, payload->filename+payload->filenameSize) << "\n";
            break;
        }
        default:
            std::cout << "DATA : " << std::string(msg->payload, msg->payload+msg->len) << "\n";
            break;
    }

    std::cout << "=== MESSAGE END ===\n";
}



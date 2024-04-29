#include "Messages.h"
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <map>
#include <fstream>

std::map<int, std::string> msgTypeNames = {
    {MSG_AUTH, "MSG_AUTH"},
    {MSG_SYNC, "MSG_SYNC"},
    {MSG_OK, "MSG_OK"},
    {MSG_ERROR, "MSG_ERROR"},
    {MSG_UPLOAD, "MSG_UPLOAD"},
    {MSG_DOWNLOAD, "MSG_DOWNLOAD"},
    {MSG_LIST_SERVER, "MSG_LIST_SERVER"},
    {MSG_FILE_ID, "MSG_FILE_ID"},
    {MSG_FILEPART, "MSG_FILEPART"},
};

int readMessage(int sock_fd, Message* msg) {
    char buffer[sizeof(Message)];
    int readBytes = 0;
    while (readBytes < sizeof(Message)) {
        int read = recv(sock_fd, buffer + readBytes, sizeof(Message) - readBytes, 0);
        if (read == 0)
            return -1;
        readBytes += read;
    };
    memcpy(msg, buffer, sizeof(Message));
    //std::cout << "Read bytes: " << readBytes << " is equal to message size?" << (sizeof(Message) == readBytes) << "\n";
    return 0;
}

void sendError(int sock_fd, std::string errorMsg) {
    Message reply;
    errorMsg.copy(reply.payload, MAX_PAYLOAD);
    reply.type = MSG_ERROR;
    reply.len = std::min((int)errorMsg.size() + 1, MAX_PAYLOAD);
    send(sock_fd, &reply, sizeof(reply), 0);
}

void sendOk(int sock_fd) {
    sendOk(sock_fd, nullptr, 0);
}

void sendOk(int sock_fd, char* data, u_int16_t dataLen) {
    Message reply = {
        .type=MSG_OK,
        .len=dataLen,
    };
    memcpy(reply.payload, data, dataLen);
    send(sock_fd, &reply, sizeof(reply), 0);
}

void printMsg(Message *msg) {
    std::cout << "=== MESSAGE BEGIN ===\n";
    std::cout << "TYPE    : " << msgTypeNames[msg->type] << "\n";
    std::cout << "LEN     : " << msg->len << "\n";
    std::cout << "---- PAYLOAD ----\n";

    switch (msg->type) {
    case MSG_FILE_ID: {
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

int waitForOk(int sock_fd) {
    Message reply;
    if (readMessage(sock_fd, &reply) == -1 || reply.type != MSG_OK)
        return -1;
    return 0;
}

int sendAuthMsg(int sock_fd, std::string username) {
    Message msg = {
        .type=MSG_AUTH,
        .len=(u_int16_t)(username.size() + 1),
    };
    username.copy(msg.payload, MAX_PAYLOAD);

    send(sock_fd, &msg, sizeof(msg), 0);
    return waitForOk(sock_fd);
}

int sendSyncMsg(int sock_fd) {
    Message msg = {.type=MSG_SYNC,.len=0};

    send(sock_fd, &msg, sizeof(msg), 0);
    return waitForOk(sock_fd);
}

int sendUploadMsg(int sock_fd) {
    Message msg = {.type=MSG_UPLOAD,.len=0};

    send(sock_fd, &msg, sizeof(msg), 0);
    return waitForOk(sock_fd);
}

int sendFile(int sock_fd, std::filesystem::path filePath) {
    std::ifstream fileToUpload(filePath, std::ifstream::binary);
    fileToUpload.seekg(0, fileToUpload.end);
    std::streampos fileSize = fileToUpload.tellg();
    fileToUpload.seekg(0, fileToUpload.beg);
    std::string filename = filePath.filename().string();

    unsigned int numBlocks = fileSize / MAX_PAYLOAD;
    if (fileSize % MAX_PAYLOAD > 0) {
        numBlocks++;
    }

    FileId payload = {
        .totalBlocks=numBlocks,
        .fileSize=static_cast<u_int64_t>(fileSize),
        .filenameSize=static_cast<u_int8_t>(filename.size()+1),
    };
    filename.copy(payload.filename, MAX_FILENAME);

    Message msg = {
        .type=MSG_FILE_ID,
        .len=sizeof(payload)
    };
    memcpy(msg.payload, &payload, sizeof(payload));

    send(sock_fd, &msg, sizeof(msg), 0);

    Message filePart = {.type=MSG_FILEPART};
    for (int i = 0; i < numBlocks; i++) {
        fileToUpload.read(filePart.payload, MAX_PAYLOAD);
        filePart.len = fileToUpload.gcount();
        //printMsg(&filePart);
        int sent = send(sock_fd, &filePart, sizeof(filePart), 0);
        //std::cout << "Sent " << sent << " bytes\n";
        if (sent == -1) {
            std::cout << "ERROR on block " << i << "\n";
            break;
        }
    }
    fileToUpload.close();
    return waitForOk(sock_fd);
}
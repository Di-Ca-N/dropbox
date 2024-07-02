#include "Messages.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

std::map<MsgType, std::string> msgTypeNames = {
    {MsgType::MSG_AUTH, "MSG_AUTH"},
    {MsgType::MSG_SYNC_CLIENT_TO_SERVER, "MSG_SYNC_CLIENT_TO_SERVER"},
    {MsgType::MSG_SYNC_SERVER_TO_CLIENT, "MSG_SYNC_SERVER_TO_CLIENT"},
    {MsgType::MSG_OK, "MSG_OK"},
    {MsgType::MSG_ERROR, "MSG_ERROR"},
    {MsgType::MSG_UPLOAD, "MSG_UPLOAD"},
    {MsgType::MSG_DOWNLOAD, "MSG_DOWNLOAD"},
    {MsgType::MSG_LIST_SERVER, "MSG_LIST_SERVER"},
    {MsgType::MSG_FILE_ID, "MSG_FILE_ID"},
    {MsgType::MSG_FILEPART, "MSG_FILEPART"},
    {MsgType::MSG_FILE_OPERATION, "MSG_FILE_OPERATION"},
    {MsgType::MSG_NUM_FILES, "MSG_NUM_FILES"},
    {MsgType::MSG_FILE_METADATA, "MSG_FILE_METADATA"},
};

std::string toString(MsgType type) { return msgTypeNames[type]; }

Message receiveMessage(int sock_fd) {
    char buffer[sizeof(Message)];
    int totalRead = 0;
    while (totalRead < sizeof(Message)) {
        int readBytes =
            recv(sock_fd, buffer + totalRead, sizeof(Message) - totalRead, 0);
        if (readBytes <= 0) throw BrokenPipe();
        totalRead += readBytes;
    };
    Message *msg = reinterpret_cast<Message *>(buffer);
    //printMsg(msg);
    return *msg;
}

void sendMessage(int sock_fd, MsgType type, const void *msgPayload,
                 u_int16_t payloadLen) {
    if (payloadLen > MAX_PAYLOAD) {
        throw PayloadTooBig();
    }
    Message msg = {
        .type = type,
        .len = payloadLen,
    };
    memcpy(msg.payload, msgPayload, payloadLen);
    //printMsg(&msg);

    // Using MSG_NOSIGNAL to avoid receiving SIGPIPE when the socket is closed
    if (send(sock_fd, &msg, sizeof(msg), MSG_NOSIGNAL) == -1)
        throw BrokenPipe();
}

void printMsg(Message *msg) {
    std::cout << "=== MESSAGE BEGIN ===\n";
    std::cout << "TYPE    : " << msgTypeNames[msg->type] << "\n";
    std::cout << "LEN     : " << msg->len << "\n";
    std::cout << "---- PAYLOAD ----\n";

    switch (msg->type) {
        case MsgType::MSG_FILE_ID: {
            FileId *payload = (FileId *)msg->payload;

            std::cout << "TOTAL BLOCKS : " << payload->totalBlocks << "\n";
            std::cout << "FILE SIZE    : " << payload->fileSize << "\n";
            std::cout << "FILENAME LEN : " << unsigned(payload->filenameSize)
                      << "\n";
            std::cout << "FILENAME     : "
                      << std::string(payload->filename,
                                     payload->filename + payload->filenameSize)
                      << "\n";
            break;
        }
        default:
            std::cout << "DATA : "
                      << std::string(msg->payload, msg->payload + msg->len)
                      << "\n";
            break;
    }

    std::cout << "=== MESSAGE END ===\n";
}

// Receive a message, validating the expected type and returns its payload
// casted to the given type
template <typename T>
T receivePayload(int sock_fd, MsgType expectedType) {
    Message reply = receiveMessage(sock_fd);
    if (reply.type != expectedType)
        throw UnexpectedMsgType(expectedType, reply.type);
    T value;
    memcpy(&value, reply.payload, reply.len);
    return value;
}

void sendOk(int sock_fd) { sendMessage(sock_fd, MsgType::MSG_OK, nullptr, 0); }

void sendError(int sock_fd, std::string errorMsg) {
    sendMessage(sock_fd, MsgType::MSG_ERROR, errorMsg.data(),
                errorMsg.length() + 1);
}

void waitConfirmation(int sock_fd) {
    Message msg = receiveMessage(sock_fd);

    // printMsg(&msg);

    if (msg.type != MsgType::MSG_OK) {
        if (msg.type == MsgType::MSG_ERROR) {
            throw ErrorReply(std::string(msg.payload, msg.payload + msg.len));
        }
        throw UnexpectedMsgType(MsgType::MSG_OK, msg.type);
    }
}

void sendAuth(int sock_fd, std::string username) {
    sendMessage(sock_fd, MsgType::MSG_AUTH, username.data(),
                username.length() + 1);
}

std::string receiveAuth(int sock_fd) {
    Message msg = receiveMessage(sock_fd);
    // printMsg(&msg);
    if (msg.type != MsgType::MSG_AUTH)
        throw UnexpectedMsgType(MsgType::MSG_AUTH, msg.type);
    return std::string(msg.payload, msg.payload + msg.len);
}

void sendFileId(int sock_fd, FileId fileId) {
    sendMessage(sock_fd, MsgType::MSG_FILE_ID, &fileId, sizeof(fileId));
}

FileId receiveFileId(int sock_fd) {
    return receivePayload<FileId>(sock_fd, MsgType::MSG_FILE_ID);
}

void sendFileData(int sock_fd, int numBlocks, std::ifstream &fileStream) {
    char buffer[MAX_PAYLOAD];
    for (int i = 0; i < numBlocks; i++) {
        fileStream.read(buffer, MAX_PAYLOAD);
        sendMessage(sock_fd, MsgType::MSG_FILEPART, buffer,
                    fileStream.gcount());
    }
}

void receiveFileData(int sock_fd, int numBlocks, std::ofstream &fileStream) {

    for (int i = 0; i < numBlocks; i++) {
        Message msg = receiveMessage(sock_fd);
        if (msg.type != MsgType::MSG_FILEPART)
            throw UnexpectedMsgType(MsgType::MSG_FILEPART, msg.type);
        fileStream.write(msg.payload, msg.len);
    }
}

void sendNumFiles(int sock_fd, int numFiles) {
    sendMessage(sock_fd, MsgType::MSG_NUM_FILES, &numFiles, sizeof(numFiles));
}

int receiveNumFiles(int sock_fd) {
    return receivePayload<int>(sock_fd, MsgType::MSG_NUM_FILES);
}

void sendFileMeta(int sock_fd, FileMeta meta) {
    sendMessage(sock_fd, MsgType::MSG_FILE_METADATA, &meta, sizeof(meta));
}

FileMeta receiveFileMeta(int sock_fd) {
    return receivePayload<FileMeta>(sock_fd, MsgType::MSG_FILE_METADATA);
}

void sendFileOperation(int sock_fd, FileOpType opType) {
    sendMessage(sock_fd, MsgType::MSG_FILE_OPERATION, &opType, sizeof(opType));
}

FileOpType receiveFileOperation(int sock_fd) {
    return receivePayload<FileOpType>(sock_fd, MsgType::MSG_FILE_OPERATION);
}

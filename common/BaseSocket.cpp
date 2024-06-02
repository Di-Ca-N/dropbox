#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>

#include "BaseSocket.hpp"

BaseSocket::BaseSocket() {
    status.reset();

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw SocketCreationError("Couldn't create socket");
}

BaseSocket::~BaseSocket() {
    close(sockfd);
}

void BaseSocket::sendOk() {
    sendMessage(MsgType::MSG_OK, nullptr, 0);
}

void BaseSocket::sendError(const std::string &errorMsg) {
    sendMessage(MsgType::MSG_ERROR, errorMsg.data(),
                errorMsg.length() + 1);
}

void BaseSocket::waitConfirmation() {
    Message msg = receiveMessage();

    // printMsg(&msg);

    if (msg.type != MsgType::MSG_OK) {
        if (msg.type == MsgType::MSG_ERROR) {
            throw ErrorReply(std::string(msg.payload, msg.payload + msg.len));
        }
        throw UnexpectedMsgTypeException(MsgType::MSG_OK, msg.type);
    }
}

void BaseSocket::sendAuth(std::string &username) {
    sendMessage(MsgType::MSG_AUTH, username.data(),
                username.length() + 1);
}

std::string BaseSocket::receiveAuth() {
    Message msg = receiveMessage();
    // printMsg(&msg);
    if (msg.type != MsgType::MSG_AUTH)
        throw UnexpectedMsgTypeException(MsgType::MSG_AUTH, msg.type);
    return std::string(msg.payload, msg.payload + msg.len);
}

void BaseSocket::sendFileId(FileId &fileId) {
    sendMessage(MsgType::MSG_FILE_ID, &fileId, sizeof(fileId));
}

FileId BaseSocket::receiveFileId() {
    return receivePayload<FileId>(MsgType::MSG_FILE_ID);
}

void BaseSocket::sendFileData(int numBlocks, std::ifstream &fileStream) {
    char buffer[MAX_PAYLOAD];
    for (int i = 0; i < numBlocks; i++) {
        fileStream.read(buffer, MAX_PAYLOAD);
        sendMessage(MsgType::MSG_FILEPART, buffer,
                    fileStream.gcount());
    }
}

void BaseSocket::receiveFileData(int numBlocks, std::ofstream &fileStream) {
    for (int i = 0; i < numBlocks; i++) {
        Message msg = receiveMessage();
        if (msg.type != MsgType::MSG_FILEPART)
            throw UnexpectedMsgTypeException(MsgType::MSG_FILEPART, msg.type);
        fileStream.write(msg.payload, msg.len);
    }
}

void BaseSocket::sendNumFiles(int numFiles) {
    sendMessage(MsgType::MSG_NUM_FILES, &numFiles, sizeof(numFiles));
}

int BaseSocket::receiveNumFiles() {
    return receivePayload<int>(MsgType::MSG_NUM_FILES);
}

void BaseSocket::sendFileMeta(FileMeta &meta) {
    sendMessage(MsgType::MSG_FILE_METADATA, &meta, sizeof(meta));
}

FileMeta BaseSocket::receiveFileMeta() {
    return receivePayload<FileMeta>(MsgType::MSG_FILE_METADATA);
}

void BaseSocket::sendFileOperation(FileOpType &opType) {
    sendMessage(MsgType::MSG_FILE_OPERATION, &opType, sizeof(opType));
}

FileOpType BaseSocket::receiveFileOperation() {
    return receivePayload<FileOpType>(MsgType::MSG_FILE_METADATA);
}

template <typename T>
T BaseSocket::receivePayload(MsgType expectedType) {
    Message reply = receiveMessage();
    if (reply.type != expectedType)
        throw UnexpectedMsgTypeException(expectedType, reply.type);
    T value;
    memcpy(&value, reply.payload, reply.len);
    return value;
}

void BaseSocket::setSynced(bool value) {
    status.set(2, value);
}

bool BaseSocket::isSynced() {
    return status[2];
}

Message BaseSocket::receiveMessage() {
    char buffer[sizeof(Message)];
    int totalRead = 0;
    while (totalRead < sizeof(Message)) {
        int readBytes =
            recv(sockfd, buffer + totalRead, sizeof(Message) - totalRead, 0);
        if (readBytes <= 0) throw BrokenPipeException("");
        totalRead += readBytes;
    };
    Message *msg = reinterpret_cast<Message *>(buffer);
    //printMsg(msg);
    return *msg;
}

void BaseSocket::sendMessage(MsgType type, const void *msgPayload,
                 u_int16_t payloadLen) {
    if (payloadLen > MAX_PAYLOAD) {
        throw PayloadTooBigException("");
    }
    Message msg = {
        .type = type,
        .len = payloadLen,
    };
    memcpy(msg.payload, msgPayload, payloadLen);
    //printMsg(&msg);

    // Using MSG_NOSIGNAL to avoid receiving SIGPIPE when the socket is closed
    if (send(sockfd, &msg, sizeof(msg), MSG_NOSIGNAL) == -1)
        throw BrokenPipeException("");
}


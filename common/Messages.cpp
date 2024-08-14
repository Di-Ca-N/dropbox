#include "Messages.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <map>

std::map<MsgType, std::string> msgTypeNames = {
    {MsgType::MSG_AUTH, "MSG_AUTH"},
    {MsgType::MSG_BALLOT, "MSG_BALLOT"},
    {MsgType::MSG_DELETE, "MSG_DELETE"},
    {MsgType::MSG_DIR_NAME, "MSG_DIR_NAME"},
    {MsgType::MSG_DOWNLOAD, "MSG_DOWNLOAD"},
    {MsgType::MSG_ELECTED, "MSG_ELECTED"},
    {MsgType::MSG_ELECTION, "MSG_ELECTION"},
    {MsgType::MSG_ERROR, "MSG_ERROR"},
    {MsgType::MSG_FILEPART, "MSG_FILEPART"},
    {MsgType::MSG_FILE_ID, "MSG_FILE_ID"},
    {MsgType::MSG_FILE_METADATA, "MSG_FILE_METADATA"},
    {MsgType::MSG_FILE_OPERATION, "MSG_FILE_OPERATION"},
    {MsgType::MSG_HEARTBEAT, "MSG_HEARTBEAT"},
    {MsgType::MSG_LIST_SERVER, "MSG_LIST_SERVER"},
    {MsgType::MSG_NUM_FILES, "MSG_NUM_FILES"},
    {MsgType::MSG_OK, "MSG_OK"},
    {MsgType::MSG_REPLICA_DATA, "MSG_REPLICA_DATA"},
    {MsgType::MSG_REPLICA_ID, "MSG_REPLICA_ID"},
    {MsgType::MSG_REPLICA_SYNC, "MSG_REPLICA_SYNC"},
    {MsgType::MSG_REPLICATION, "MSG_REPLICATION"},
    {MsgType::MSG_SERVER_ADDRESS, "MSG_SERVER_ADDRESS"},
    {MsgType::MSG_SERVICE_STATUS, "MSG_SERVICE_STATUS"},
    {MsgType::MSG_STATUS_INQUIRY, "MSG_STATUS_INQUIRY"},
    {MsgType::MSG_SYNC_CLIENT_TO_SERVER, "MSG_SYNC_CLIENT_TO_SERVER"},
    {MsgType::MSG_SYNC_SERVER_TO_CLIENT, "MSG_SYNC_SERVER_TO_CLIENT"},
    {MsgType::MSG_UPLOAD, "MSG_UPLOAD"},
    {MsgType::MSG_UPDATE_TYPE, "MSG_UPDATE_TYPE"},
};

std::string toString(MsgType type) { return msgTypeNames[type]; }

bool operator==(ServerAddress addr1, ServerAddress addr2) {
    return addr1.ip == addr2.ip && addr1.port == addr2.port;
}

std::ostream &operator<<(std::ostream &os, const ServerAddress &addr) {
    char ipString[16];
    inet_ntop(AF_INET, &addr.ip, ipString, 16);
    os << ipString << ":" << ntohs(addr.port);
    return os;
}

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
    if (reply.type == MsgType::MSG_ERROR)
        throw ErrorReply(std::string(reply.payload, reply.len));
    if (reply.type != expectedType)
        throw UnexpectedMsgType(expectedType, reply.type);
    T value;
    memcpy(&value, reply.payload, reply.len);
    return value;
}

void sendOk(int sock_fd) { sendMessage(sock_fd, MsgType::MSG_OK, nullptr, 0); }

void sendError(int sock_fd, std::string errorMsg) {
    sendMessage(sock_fd, MsgType::MSG_ERROR, errorMsg.data(),
                errorMsg.length());
}

void waitConfirmation(int sock_fd) {
    Message msg = receiveMessage(sock_fd);

    if (msg.type != MsgType::MSG_OK) {
        if (msg.type == MsgType::MSG_ERROR) {
            throw ErrorReply(std::string(msg.payload, msg.payload + msg.len));
        }
        throw UnexpectedMsgType(MsgType::MSG_OK, msg.type);
    }
}

void sendAuth(int sock_fd, AuthData authData) {
    sendMessage(sock_fd, MsgType::MSG_AUTH, &authData, sizeof(authData));
}

AuthData receiveAuth(int sock_fd) {
    return receivePayload<AuthData>(sock_fd, MsgType::MSG_AUTH);
}

FileId getFileId(std::filesystem::path target) {
    std::string fileName;
    FileId fileData;

    fileData.fileSize = std::filesystem::file_size(target);

    fileData.totalBlocks = fileData.fileSize / MAX_PAYLOAD;
    if (fileData.fileSize % MAX_PAYLOAD > 0)
        fileData.totalBlocks++;

    fileName = target.filename();

    fileData.filenameSize = fileName.size();

    strncpy(fileData.filename, fileName.c_str(), fileData.filenameSize); 

    return fileData;
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

void sendServerAddress(int sock_fd, ServerAddress address) {
    sendMessage(sock_fd, MsgType::MSG_SERVER_ADDRESS, &address, sizeof(address));
}

ServerAddress receiveServerAddress(int sock_fd) {
    return receivePayload<ServerAddress>(sock_fd, MsgType::MSG_SERVER_ADDRESS);
}

void sendHeartbeat(int sock_fd) {
    sendMessage(sock_fd, MsgType::MSG_HEARTBEAT, nullptr, 0);
}

void waitHeartbeat(int sock_fd, int maxTimeout) {
    Message msg = receiveMessage(sock_fd);
    switch (msg.type) {
        case MsgType::MSG_HEARTBEAT: return;
        case MsgType::MSG_ERROR: throw ErrorReply(std::string(msg.payload, msg.payload + msg.len));
        default: throw UnexpectedMsgType(MsgType::MSG_HEARTBEAT, msg.type);
    }
}

void sendUpdateType(int sock_fd, UpdateType updateType) {
    sendMessage(sock_fd, MsgType::MSG_UPDATE_TYPE, &updateType, sizeof(updateType));
}

UpdateType receiveUpdateType(int sock_fd) {
    return receivePayload<UpdateType>(sock_fd, MsgType::MSG_UPDATE_TYPE);
}

void sendReplicaData(int sock_fd, ReplicaData replicaData) {
    sendMessage(sock_fd, MsgType::MSG_REPLICA_DATA, &replicaData, sizeof(replicaData));
}

ReplicaData receiveReplicaData(int sock_fd) {
    return receivePayload<ReplicaData>(sock_fd, MsgType::MSG_REPLICA_DATA);
}

void sendBallot(int sock_fd, Ballot ballot) {
    sendMessage(sock_fd, MsgType::MSG_BALLOT, &ballot, sizeof(ballot));
}

Ballot receiveBallot(int sock_fd) {
    return receivePayload<Ballot>(sock_fd, MsgType::MSG_BALLOT);
}
void sendReplicaId(int sock_fd, int replicaId) {
    sendMessage(sock_fd, MsgType::MSG_REPLICA_ID, &replicaId, sizeof(replicaId));
}

int receiveReplicaId(int sock_fd) {
    return receivePayload<int>(sock_fd, MsgType::MSG_REPLICA_ID);
}

void sendDirName(int sock_fd, DirData dirName) {
    sendMessage(sock_fd, MsgType::MSG_DIR_NAME, &dirName, sizeof(dirName));
}

DirData receiveDirName(int sock_fd) {
    return receivePayload<DirData>(sock_fd, MsgType::MSG_DIR_NAME);
}


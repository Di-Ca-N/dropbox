#include "Messages.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>

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

int receiveMessage(int sock_fd, Message *msg) {
    char buffer[sizeof(Message)];
    int totalRead = 0;
    while (totalRead < sizeof(Message)) {
        int readBytes =
            recv(sock_fd, buffer + totalRead, sizeof(Message) - totalRead, 0);
        if (readBytes == 0) return ERROR_BROKEN_PIPE;
        totalRead += readBytes;
    };
    memcpy(msg, buffer, sizeof(Message));
    // std::cout << "Read bytes: " << readBytes << " is equal to message size?"
    // << (sizeof(Message) == readBytes) << "\n";
    return 0;
}

int sendMessage(int sock_fd, MsgType type, const void *msgPayload,
                unsigned int payloadLen) {
    if (payloadLen > MAX_PAYLOAD) {
        return ERROR_PAYLOAD_TOO_BIG;
    }
    Message msg = {
        .type = type,
        .len = payloadLen,
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

/* Reads a message. If the type has the expected type, copies at most destSize
bytes from the payload into *dest and returns 0. Optionally, you may pass a pointer
to access the raw read message.
*/
int receivePayload(int sock_fd, MsgType expectedType, void *dest, size_t destSize) {
    Message reply;
    int err = receiveMessage(sock_fd, &reply);
    if (err < 0) return err;
    if (reply.type != expectedType) return ERROR_UNEXPECTED_MSG_TYPE;
    memcpy(dest, reply.payload, destSize);
    return 0;
}

int sendOk(int sock_fd) {
    return sendMessage(sock_fd, MsgType::MSG_OK, nullptr, 0);
}

int sendError(int sock_fd, std::string errorMsg) {
    return sendMessage(sock_fd, MsgType::MSG_ERROR, errorMsg.data(),
                       errorMsg.length() + 1);
}

int waitForOk(int sock_fd, Message *replyPtr) {
    Message msg;
    int err = receiveMessage(sock_fd, &msg);
    if (err < 0) return err;

    if (replyPtr != nullptr) {
        memcpy(replyPtr, &msg, sizeof(msg));
    }

    if (msg.type != MsgType::MSG_OK) {
        if (msg.type == MsgType::MSG_ERROR) {
            return ERROR_ERROR_REPLY;
        }
        return ERROR_UNEXPECTED_MSG_TYPE;
    }

    return 0;
}

int sendFileId(int sock_fd, FileId fileId) {
    return sendMessage(sock_fd, MsgType::MSG_FILE_ID, &fileId, sizeof(fileId));
}

int receiveFileId(int sock_fd, FileId *fileId) {
    return receivePayload(sock_fd, MsgType::MSG_FILE_ID, fileId,
                          sizeof(FileId));
}

int sendFileData(int sock_fd, int numBlocks, std::ifstream &fileStream) {
    char buffer[MAX_PAYLOAD];
    for (int i = 0; i < numBlocks; i++) {
        fileStream.read(buffer, MAX_PAYLOAD);
        int err = sendMessage(sock_fd, MsgType::MSG_FILEPART, buffer,
                              fileStream.gcount());
        if (err < 0) return err;
    }
    return 0;
}

int receiveFileData(int sock_fd, int numBlocks, std::ofstream &fileStream) {
    Message msg;
    for (int i = 0; i < numBlocks; i++) {
        int err = receiveMessage(sock_fd, &msg);
        if (err < 0) return err;

        if (msg.type != MsgType::MSG_FILEPART) return ERROR_UNEXPECTED_MSG_TYPE;
        fileStream.write(msg.payload, msg.len);
    }
    return 0;
}

int sendNumFiles(int sock_fd, int numFiles) {
    return sendMessage(sock_fd, MsgType::MSG_NUM_FILES, &numFiles, sizeof(numFiles));
}

int receiveNumFiles(int sock_fd, int *numFiles) {
    return receivePayload(sock_fd, MsgType::MSG_NUM_FILES, numFiles, sizeof(*numFiles));
}

int sendFileMeta(int sock_fd, FileMeta meta) { 
    return sendMessage(sock_fd, MsgType::MSG_FILE_METADATA, &meta, sizeof(meta));
}

int receiveFileMeta(int sock_fd, FileMeta *meta) {
    return receivePayload(sock_fd, MsgType::MSG_FILE_METADATA, meta, sizeof(*meta));
}

int sendFileOperation(int sock_fd, FileOpType opType) {
    return sendMessage(sock_fd, MsgType::MSG_FILE_OPERATION, &opType, sizeof(opType));
}

int receiveFileOperation(int sock_fd, FileOpType *opType) {
    return receivePayload(sock_fd, MsgType::MSG_FILE_METADATA, opType, sizeof(*opType));
}

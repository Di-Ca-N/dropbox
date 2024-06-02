// This Socket wrapper ensures consistency in resource acquisition and release

#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include <bitset>
#include <stdexcept>

#include "Messages.hpp"

class BaseSocket {
private:
    // Receive a message, validating the expected type and returns its payload
    // casted to the given type
    template <typename T>
    T receivePayload(MsgType expectedType);

protected:
    int sockfd;
    std::bitset<3> status; // BIND, LISTEN, ACCEPT/CONNECT

    BaseSocket();
    ~BaseSocket();
    void setSynced(bool value);
    bool isSynced();

public:
    Message receiveMessage();
    void sendMessage(MsgType type, const void* msgPayload,
                     u_int16_t payloadLen);
    void sendOk();
    void sendError(const std::string &errorMsg);
    void waitConfirmation();
    void sendAuth(std::string &username);
    std::string receiveAuth();
    void sendFileId(FileId &fileId);
    FileId receiveFileId();
    void sendFileData(int numBlocks, std::ifstream &fileStream);
    void receiveFileData(int numBlocks, std::ofstream &fileStream);
    void sendNumFiles(int numFiles);
    int receiveNumFiles();
    void sendFileMeta(FileMeta &meta);
    FileMeta receiveFileMeta();
    void sendFileOperation(FileOpType &type);
    FileOpType receiveFileOperation();
};

class SocketCreationError : public std::runtime_error {
public:
    SocketCreationError(const std::string &message)
        : std::runtime_error(message) {}
};

class SocketSyncError : public std::runtime_error {
public:
    SocketSyncError(const std::string &message)
        : std::runtime_error(message) {}
};

class SocketIOError : public std::runtime_error {
public:
    SocketIOError(const std::string &message)
        : std::runtime_error(message) {}
};

/* ======== EXCEPTIONS ======= */
// Can be thrown by any function if you try to use a closed socket
class BrokenPipeException : public std::runtime_error {
   public:
    BrokenPipeException(const std::string &message)
        : std::runtime_error(message) {};
};

// Throw by the High Level functions when the received message type
// does not match the expected type
class UnexpectedMsgTypeException : public std::runtime_error {
    private:
        static std::string formatMessage(
                const MsgType &expected,
                const MsgType &received) {
            return "Got unexpected message type: " + toString(received) +
                  " (expected: " + toString(expected) + ")";
        }

   public:
    UnexpectedMsgTypeException(MsgType expected, MsgType received)
        : std::runtime_error(formatMessage(expected, received)) {};
};

// Thrown only by waitConfirmation, if the other side replies with a message
// with type MSG_ERROR.
class ErrorReply : public std::runtime_error {
   public:
    ErrorReply(const std::string &message)
        : std::runtime_error(message) {};
};

// Thrown only by sendMessage. If you are only using the High-level API,
// you should never see this exception
class PayloadTooBigException : public std::runtime_error {
   public:
    PayloadTooBigException(const std::string &message)
        : std::runtime_error(message) {};
};

#endif

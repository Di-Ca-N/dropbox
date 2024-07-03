#pragma once

#include <sys/types.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

#define MAX_PAYLOAD 512
#define MAX_FILENAME 256
#define MAX_USERNAME 256

/* ====== DATA DEFINITIONS ====== */

enum class MsgType : u_int8_t {
    MSG_AUTH,
    MSG_SYNC_CLIENT_TO_SERVER,
    MSG_SYNC_SERVER_TO_CLIENT,
    MSG_UPLOAD,
    MSG_DOWNLOAD,
    MSG_DELETE,
    MSG_LIST_SERVER,
    MSG_FILE_ID,
    MSG_FILEPART,
    MSG_FILE_OPERATION,
    MSG_NUM_FILES,
    MSG_FILE_METADATA,
    MSG_OK,
    MSG_ERROR,
    MSG_DEVICE_ID,
    MSG_REGISTER_DEVICE,
};

// Helper function to get a string representation of MsgType
std::string toString(MsgType type);

// Base Message. The Payload content depends on the MsgType.
typedef struct {
    MsgType type;
    u_int16_t len;
    char payload[MAX_PAYLOAD];
} Message;

// Struct to identify a file for transference. When sending a file, all fields must be
// filled. When requesting a file, only the filename and filenameSize are required.
typedef struct {
    u_int64_t totalBlocks;
    u_int64_t fileSize;
    u_int8_t filenameSize;
    char filename[MAX_FILENAME];
} FileId;

enum class FileOpType : u_int8_t { FILE_MODIFY, FILE_DELETE, FILE_MOVE };

// Struct to identify a file operation
typedef struct {
    FileOpType type;
    u_int8_t filenameSize;
    char filename[MAX_FILENAME];
} FileOperation;

// Struct to transfer file metadata
typedef struct {
    char filename[MAX_FILENAME];
    time_t mTime;
    time_t aTime;
    time_t cTime;
} FileMeta;

/* =========== HIGH-LEVEL API ============= */
void sendOk(int sock_fd);
void sendError(int sock_fd, std::string errorMsg);
void waitConfirmation(int sock_fd);
void sendAuth(int sock_fd, std::string username);
std::string receiveAuth(int sock_fd);
void sendDeviceId(int sock_fd);
int receiveDeviceId(int sock_fd);
FileId getFileId(std::filesystem::path target);
void sendFileId(int sock_fd, FileId fileId);
FileId receiveFileId(int sock_fd);
void sendFileData(int sock_fd, int numBlocks, std::ifstream& fileStream);
void receiveFileData(int sock_fd, int numBlocks, std::ofstream& fileStream);
void sendNumFiles(int sock_fd, int numFiles);
int receiveNumFiles(int sock_fd);
void sendFileMeta(int sock_fd, FileMeta meta);
FileMeta receiveFileMeta(int sock_fd);
void sendFileOperation(int sock_fd, FileOpType type);
FileOpType receiveFileOperation(int sock_fd);

/* =========== LOW-LEVEL API ============= */
Message receiveMessage(int sock_fd);
void sendMessage(int sock_fd, MsgType type, const void* msgPayload,
                 u_int16_t payloadLen);
void printMsg(Message* msg);

/* ======== EXCEPTIONS ======= */
// Can be thrown by any function if you try to use a closed socket
class BrokenPipe : public std::exception {
   public:
    BrokenPipe(){};
};

// Throw by the High Level functions when the received message type
// does not match the expected type
class UnexpectedMsgType : public std::exception {
   private:
    MsgType expected;
    MsgType received;
    std::string msg;

   public:
    UnexpectedMsgType(MsgType expected, MsgType received) {
        this->expected = expected;
        this->received = received;
        msg = "Got unexpected message type: " + toString(received) +
              " (expected: " + toString(expected) + ")";
    };

    const char* what() const throw() { return msg.c_str(); }
};

// Thrown only by waitConfirmation, if the other side replies with a message
// with type MSG_ERROR.
class ErrorReply : public std::exception {
   private:
    std::string msg;

   public:
    ErrorReply(std::string msg) { this->msg = msg; };

    const char* what() const throw() { return msg.c_str(); }
};

// Thrown only by sendMessage. If you are only using the High-level API,
// you should never see this exception
class PayloadTooBig : public std::exception {
   public:
    PayloadTooBig(){};
};

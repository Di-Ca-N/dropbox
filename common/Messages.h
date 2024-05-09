#pragma once

#include <sys/types.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <exception>
#include <map>

#define MAX_PAYLOAD 512
#define MAX_FILENAME 256
#define MAX_USERNAME 256

#define ERROR_BROKEN_PIPE -1
#define ERROR_PAYLOAD_TOO_BIG -2
#define ERROR_ERROR_REPLY -3
#define ERROR_UNEXPECTED_MSG_TYPE -4

enum class MsgType : u_int8_t {
    MSG_AUTH,
    MSG_SYNC_CLIENT_TO_SERVER,
    MSG_SYNC_SERVER_TO_CLIENT,
    MSG_UPLOAD,
    MSG_DOWNLOAD,
    MSG_LIST_SERVER,
    MSG_FILE_ID,
    MSG_FILEPART,
    MSG_FILE_OPERATION,
    MSG_NUM_FILES,
    MSG_FILE_METADATA,
    MSG_OK,
    MSG_ERROR
};

std::string toString(MsgType type);

typedef struct {
    MsgType type;
    u_int16_t len;
    char payload[MAX_PAYLOAD];
} Message;

typedef struct {
    u_int32_t totalBlocks;
    u_int64_t fileSize;
    u_int8_t filenameSize;
    char filename[MAX_FILENAME];
} FileId;

enum class FileOpType : u_int8_t {
    FILE_MODIFY,
    FILE_DELETE,
    FILE_MOVE
};

typedef struct {
    FileOpType type;
} FileOp;

typedef struct {
    char filename[MAX_FILENAME];
    time_t mTime;
    time_t aTime;
    time_t cTime;
} FileMeta;

/* ======== EXCEPTIONS ======= */
class BrokenPipe : public std::exception {
    public:
        BrokenPipe() {};
};

class UnexpectedMsgType : public std::exception {
    private:
        MsgType expected;
        MsgType received;
        std::string msg;
    public:
        UnexpectedMsgType(MsgType expected, MsgType received) {
            this->expected = expected;
            this->received = received;
            msg = "Got unexpected message type: " + toString(received) + " (expected: " + toString(expected) + ")";
        };

        const char* what() const throw() { 
            return msg.c_str(); 
        } 
};

class PayloadTooBig : public std::exception {
    public:
        PayloadTooBig() {};
};

class ErrorReply : public std::exception {
    private:
        std::string msg;
    public:
        ErrorReply(std::string msg) {
            this->msg = msg;
        };

        const char* what() const throw() { 
            return msg.c_str(); 
        }
};


/* =========== HIGH LEVEL API ============= */

void sendOk(int sock_fd);
void sendError(int sock_fd, std::string errorMsg);
void waitConfirmation(int sock_fd);
void sendAuth(int sock_fd, std::string username);
std::string receiveAuth(int sock_fd);
void sendFileId(int sock_fd, FileId fileId);
FileId receiveFileId(int sock_fd);
void sendFileData(int sock_fd, int numBlocks, std::ifstream &fileStream);
void receiveFileData(int sock_fd, int numBlocks, std::ofstream &fileStream);
void sendNumFiles(int sock_fd, int numFiles);
int receiveNumFiles(int sock_fd);
void sendFileMeta(int sock_fd, FileMeta meta);
FileMeta receiveFileMeta(int sock_fd);
void sendFileOperation(int sock_fd, FileOpType type);
FileOpType receiveFileOperation(int sock_fd);

/* =========== LOW LEVEL API ============= */
Message receiveMessage(int sock_fd); 
void sendMessage(int sock_fd, MsgType type, const void* msgPayload, unsigned int payloadLen);
void printMsg(Message *msg);

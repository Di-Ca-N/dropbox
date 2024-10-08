#pragma once

#include <sys/types.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include <netinet/in.h>

#define MAX_PAYLOAD 512
#define MAX_FILENAME 256
#define MAX_USERNAME 256
#define MAX_DIRNAME 256

/* ====== DATA DEFINITIONS ====== */

enum class MsgType : u_int8_t {
    MSG_AUTH,
    MSG_BALLOT,
    MSG_DELETE,
    MSG_DIR_NAME,
    MSG_DOWNLOAD,
    MSG_ELECTED,
    MSG_ELECTION,
    MSG_ERROR,
    MSG_FILEPART,
    MSG_FILE_ID,
    MSG_FILE_METADATA,
    MSG_FILE_OPERATION,
    MSG_HEARTBEAT,
    MSG_LIST_SERVER,
    MSG_NUM_FILES,
    MSG_OK,
    MSG_REPLICA_DATA,
    MSG_REPLICA_ID,
    MSG_REPLICA_SYNC,
    MSG_REPLICATION,
    MSG_SERVER_ADDRESS,
    MSG_SERVICE_STATUS,
    MSG_STATUS_INQUIRY,
    MSG_SYNC_CLIENT_TO_SERVER,
    MSG_SYNC_SERVER_TO_CLIENT,
    MSG_UPLOAD,
    MSG_UPDATE_TYPE,
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

enum class FileOpType : u_int8_t { FILE_MODIFY, FILE_DELETE, FILE_MOVE, VOID_OP };

// Struct to identify a file operation
typedef struct {
    FileOpType type;
    u_int8_t filenameSize;
    char filename[MAX_FILENAME];
} FileOperation;

// Struct to transfer file metadata
typedef struct {        // switch (op.type) {
        //     case FileOpType::FILE_DELETE:
        //         break;
        //     case FileOpType::FILE_MODIFY:
        //         handleFileModify(replicaSock, fileName, username);
        //         break;
        //     default:
        //         break;
        // }
    char filename[MAX_FILENAME];
    time_t mTime;
    time_t aTime;
    time_t cTime;
} FileMeta;

enum class ServiceStatusType : u_int8_t { OFFLINE, ONLINE };

// Struct to identify the service status
typedef struct {
    ServiceStatusType status;
} ServiceStatus;

// Struct to identify a server address
typedef struct {
    in_addr_t ip;
    in_port_t port;
} ServerAddress;

bool operator==(ServerAddress addr1, ServerAddress addr2);
std::ostream& operator<<(std::ostream& os, const ServerAddress& addr);

// === Authentication related data types ===

// Indicates the source of the authentication request
enum class AuthType : uint8_t { AUTH_CLIENT, AUTH_REPLICA };

// Contains data required for client authentication. If the device still does not have 
// an id, the field deviceId must be set to 0.
typedef struct {
    char username[MAX_USERNAME];
    uint8_t usernameLen;
    int deviceId;
} ClientAuthData;

// Contains data required for replica authentication. The field ipAddress may be empty
// and the server will reply with the IP used for the connection.
typedef struct {
    ServerAddress replicaAddr;
    int replicaId;
} ReplicaAuthData;

// Struct with the required data to perform authentication. The field type
// specifies whether the data is related to a client or to a replica. You only 
// should access the fields related to the specified authentication type.
typedef struct {
    AuthType type;
    union {
       ClientAuthData clientData;
       ReplicaAuthData replicaData;
    };
} AuthData;

enum class UpdateType : uint8_t { UPDATE_CONNECTION, UPDATE_FILE_OP, UPDATE_CONNECTION_START, UPDATE_CONNECTION_END };

typedef struct {
    int replicaId;
    ServerAddress replicaAddr;
    int socketDescr;
} ReplicaData;

typedef struct {
    ServerAddress address;
    int id;
} Ballot;

typedef struct {
    char dirName[MAX_DIRNAME];
    uint8_t dirnameLen;
} DirData;

/* =========== HIGH-LEVEL API ============= */
/* This high-level API provide utility functions for sending and receiving each data 
 * type provided in our protocol implementation. These functions ensure that the sent 
 * and received messages contain the expected types and handle the serialization and 
 * deserialization of each data type. If this is not enough for your use-case, you may 
 * fallback to the low-level API described below.
 * */

void sendOk(int sock_fd);
void sendError(int sock_fd, std::string errorMsg);
void waitConfirmation(int sock_fd);
void sendAuth(int sock_fd, AuthData authData);
AuthData receiveAuth(int sock_fd);
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
void sendServerAddress(int sock_fd, ServerAddress address);
ServerAddress receiveServerAddress(int sock_fd);
void sendHeartbeat(int sock_fd);
void waitHeartbeat(int sock_fd, int maxTimeout);
void sendUpdateType(int sock_fd, UpdateType updateType);
UpdateType receiveUpdateType(int sock_fd);
void sendReplicaData(int sock_fd, ReplicaData replicaData);
ReplicaData receiveReplicaData(int sock_fd);
void sendReplicaId(int sock_fd, int replicaId);
int receiveReplicaId(int sock_fd);
void sendBallot(int sock_fd, Ballot ballot);
Ballot receiveBallot(int sock_fd);
void sendDirName(int sock_fd, DirData dirData);
DirData receiveDirName(int sock_fd);


/* =========== LOW-LEVEL API ============= */
/* This is the low-level API of our protocol, dealing directly with sending and receiving 
 * raw messages. This API only ensures the integrity of the received messages, and you must
 * manually handle the serialization and deserialization of its payload.
 * */

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

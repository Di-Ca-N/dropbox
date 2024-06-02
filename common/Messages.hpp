#ifndef MESSAGES_H
#define MESSAGES_H

#include <sys/types.h>
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
};

enum class FileOpType : u_int8_t { FILE_MODIFY, FILE_DELETE, FILE_MOVE };

// Base Message. The Payload content depends on the MsgType.
struct Message {
    MsgType type;
    u_int16_t len;
    char payload[MAX_PAYLOAD];
};

// Struct to identify a file for transference. When sending a file, all fields must be
// filled. When requesting a file, only the filename and filenameSize are required.
struct FileId {
    u_int64_t totalBlocks;
    u_int64_t fileSize;
    u_int8_t filenameSize;
    char filename[MAX_FILENAME];
};

// Struct to identify a file operation
struct FileOperation {
    FileOpType type;
    u_int8_t filenameSize;
    char filename[MAX_FILENAME];
};

// Struct to transfer file metadata
struct FileMeta {
    char filename[MAX_FILENAME];
    time_t mTime;
    time_t aTime;
    time_t cTime;
};

/* ======== FUNCTIONS ======= */
std::string toString(MsgType type);
void printMsg(Message &msg);
void printMeta(FileMeta &meta);
u_int64_t getNumBlocks(u_int64_t fileSize, u_int64_t blockSize);

#endif

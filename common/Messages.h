#include <sys/types.h>
#include <string>
#include <filesystem>

#define MAX_PAYLOAD 512
#define MAX_FILENAME 256

#define ERROR_BROKEN_PIPE -1
#define ERROR_PAYLOAD_TOO_BIG -2

enum class MsgType : u_int8_t {
    MSG_AUTH,
    MSG_SYNC,
    MSG_UPLOAD,
    MSG_DOWNLOAD,
    MSG_LIST_SERVER,
    MSG_FILE_ID,
    MSG_FILEPART,
    MSG_FILE_OPERATION,
    MSG_NUM_FILES,
    MSG_OK,
    MSG_ERROR
};

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


/* =========== HIGH LEVEL API =============
 * The following functions compose the high-level API of our protocol.
 * These functions use the more basic functions below to implement
 * their functionality.
 * =======================================
 */

// ===== Functions for sending/receiving responses =====

/* Send an OK message. Return 0 on success or a negative number on error. */
int sendOk(int sock_fd);

/* Send an ERROR message. Return 0 on success or a negative number on error. */
int sendError(int sock_fd, std::string errorMsg);

/* Wait to receive an OK message. Return 0 on success or a negative number on error. 
If you want the given response, you can optionally pass a pointer where the response will be saved.
*/
int waitForOk(int sock_fd, Message *reply = nullptr);


// ===== Functions to handle data exchange between client/server =====
// All these functions come in send/receive pairs.

/* Send the informed FileId. Return 0 on success or a negative number on error. */
int sendFileId(int sock_fd, FileId fileId);

/* Receive a FileId. Return 0 on success or a negative number on error. */
int receiveFileId(int sock_fd, FileId *fileId);

/* Read file data from fileStream and sends it through sock_fd. Return 0 on success or a negative number on error.*/
int sendFileData(int sock_fd, std::ifstream &fileStream);

/* Receive file data from sock_fd and and writes it to fileStream. Return 0 on success or a negative number on error. */
int receiveFileData(int sock_fd, std::ofstream &fileStream);

/* Send the quantity of files that will be listed. Return 0 on success or a negative number on error. */
int sendNumFiles(int sock_fd, int numFiles);

/* Receive the quantity of files that will be listed. Return 0 on success or a negative number on error. */
int receiveNumFiles(int sock_fd, int *numFiles);

/* Send the metadata information of one file. Return 0 on success or a negative number on error. */
int sendFileMeta(int sock_fd, FileMeta meta);

/* Receive the metadata information of one file. Return 0 on success or a negative number on error. */
int receiveFileMeta(int sock_fd, FileMeta *meta);

/* Send one FileOperation message with the given type. Return 0 on success or a negative number on error. */
int sendFileOperation(int sock_fd, FileOpType type);

/* Receive one FileOperation message. Return 0 on success or a negative number on error. */
int receiveFileOperation(int sock_fd, FileOpType *type);


/* =========== LOW LEVEL API =============
 * The following functions compose the low-level API of our protocol.
 * If possible, use the high level functions declared above.
 * =======================================
 */

/* Read one message from sock_fd into *msg. 
This function ensures that the read message is always be complete.
Return 0 on success and a negative number on error. On error, the final value on *msg is undefined */
int receiveMessage(int sock_fd, Message *msg); 

/* Send a message with the given type and payload into fd. 
Return 0 on success and a negative number on error */
int sendMessage(int sock_fd, MsgType type, void* msgPayload, unsigned int payloadLen);

/* Print a message on stdout */
void printMsg(Message *msg);

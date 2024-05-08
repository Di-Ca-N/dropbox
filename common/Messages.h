#include <sys/types.h>
#include <string>

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

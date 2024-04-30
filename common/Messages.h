#include <sys/types.h>
#include <string>
#include <filesystem>

#define MAX_PAYLOAD 512
#define MAX_FILENAME 256

#define MSG_AUTH 0
#define MSG_SYNC 1
#define MSG_UPLOAD 2
#define MSG_DOWNLOAD 3
#define MSG_LIST_SERVER 4
#define MSG_FILE_ID 5
#define MSG_FILEPART 6
#define MSG_OK 254
#define MSG_ERROR 255


typedef struct {
    u_int8_t type;
    u_int16_t len;
    char payload[MAX_PAYLOAD];
} Message;

typedef struct {
    u_int32_t totalBlocks;
    u_int64_t fileSize;
    u_int8_t filenameSize;
    char filename[MAX_FILENAME];
} FileId;


int readMessage(int sock_fd, Message* msg);
void sendError(int sock_fd, std::string errorMsg);
int sendOk(int sock_fd);
int sendOk(int sock_fd, char* data, u_int16_t dataLen);
void printMsg(Message *msg);
int sendAuthMsg(int sock_fd, std::string username);
int sendSyncMsg(int sock_fd);
int sendUploadMsg(int sock_fd);
int sendDownloadMsg(int sock_fd);
int sendFileId(int sock_fd, std::string filename, int numBlocks, u_int64_t fileSize);
int sendFile(int sock_fd, std::filesystem::path filePath);
int receiveFile(int sock_fd, std::filesystem::path filePath, int totalBlocks);
int receiveFileId(int sock_fd, FileId *fileId);

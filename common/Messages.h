#include <sys/types.h>
#include <string>
#include <filesystem>
#include <stdexcept>

#define MAX_PAYLOAD 512
#define MAX_FILENAME 256


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


Message readMessage(int sock_fd);
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

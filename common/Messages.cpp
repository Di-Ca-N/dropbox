#include "Messages.h"
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <map>
#include <fstream>
#include <stdexcept>

std::map<MsgType, std::string> msgTypeNames = {
    {MsgType::MSG_AUTH, "MSG_AUTH"},
    {MsgType::MSG_SYNC, "MSG_SYNC"},
    {MsgType::MSG_OK, "MSG_OK"},
    {MsgType::MSG_ERROR, "MSG_ERROR"},
    {MsgType::MSG_UPLOAD, "MSG_UPLOAD"},
    {MsgType::MSG_DOWNLOAD, "MSG_DOWNLOAD"},
    {MsgType::MSG_LIST_SERVER, "MSG_LIST_SERVER"},
    {MsgType::MSG_FILE_ID, "MSG_FILE_ID"},
    {MsgType::MSG_FILEPART, "MSG_FILEPART"},
};

Message readMessage(int sock_fd) {
    char buffer[sizeof(Message)];
    int readBytes = 0;
    while (readBytes < sizeof(Message)) {
        int read = recv(sock_fd, buffer + readBytes, sizeof(Message) - readBytes, 0);
        if (read == 0)
            throw std::runtime_error("broken pipe");
        readBytes += read;
    };
    Message *msg = reinterpret_cast<Message*>(buffer);
    return *msg;
}

void sendError(int sock_fd, std::string errorMsg) {
    Message reply;
    errorMsg.copy(reply.payload, MAX_PAYLOAD);
    reply.type = MsgType::MSG_ERROR;
    reply.len = std::min((int)errorMsg.size() + 1, MAX_PAYLOAD);
    send(sock_fd, &reply, sizeof(reply), 0);
}

int sendOk(int sock_fd) {
    return sendOk(sock_fd, nullptr, 0);
}

int sendOk(int sock_fd, char* data, u_int16_t dataLen) {
    Message reply = {
        .type=MsgType::MSG_OK,
        .len=dataLen,
    };
    memcpy(reply.payload, data, dataLen);
    if (send(sock_fd, &reply, sizeof(reply), MSG_NOSIGNAL) == -1) 
        return -1;
    return 0;
}

void printMsg(Message *msg) {
    std::cout << "=== MESSAGE BEGIN ===\n";
    std::cout << "TYPE    : " << msgTypeNames[msg->type] << "\n";
    std::cout << "LEN     : " << msg->len << "\n";
    std::cout << "---- PAYLOAD ----\n";

    switch (msg->type) {
    case MsgType::MSG_FILE_ID: {
        FileId *payload = (FileId*) msg->payload;

        std::cout << "TOTAL BLOCKS : " << payload->totalBlocks << "\n";
        std::cout << "FILE SIZE    : " << payload->fileSize << "\n";
        std::cout << "FILENAME LEN : " << unsigned(payload->filenameSize) << "\n";
        std::cout << "FILENAME     : " << std::string(payload->filename, payload->filename+payload->filenameSize) << "\n";
        break;
    }
    default:
        std::cout << "DATA : " << std::string(msg->payload, msg->payload+msg->len) << "\n";
        break;
    }
    
    std::cout << "=== MESSAGE END ===\n";
}

int waitForOk(int sock_fd) {
    Message reply = readMessage(sock_fd);
    if (reply.type != MsgType::MSG_OK)
        return -1;
    return 0;
}

int sendAuthMsg(int sock_fd, std::string username) {
    Message msg = {
        .type=MsgType::MSG_AUTH,
        .len=(u_int16_t)(username.size() + 1),
    };
    username.copy(msg.payload, MAX_PAYLOAD);

    send(sock_fd, &msg, sizeof(msg), 0);
    return waitForOk(sock_fd);
}

int sendSyncMsg(int sock_fd) {
    Message msg = {.type=MsgType::MSG_SYNC,.len=0};

    send(sock_fd, &msg, sizeof(msg), 0);
    return waitForOk(sock_fd);
}

int sendUploadMsg(int sock_fd) {
    Message msg = {.type=MsgType::MSG_UPLOAD,.len=0};

    send(sock_fd, &msg, sizeof(msg), 0);
    return waitForOk(sock_fd);
}

int sendDownloadMsg(int sock_fd) {
    Message msg = {.type=MsgType::MSG_DOWNLOAD,.len=0};

    send(sock_fd, &msg, sizeof(msg), 0);
    return waitForOk(sock_fd);
}

int sendFileId(int sock_fd, std::string filename, int numBlocks, u_int64_t fileSize) {
    FileId payload = {
        .totalBlocks=numBlocks,
        .fileSize=static_cast<u_int64_t>(fileSize),
        .filenameSize=static_cast<u_int8_t>(filename.size()+1),
    };
    filename.copy(payload.filename, MAX_FILENAME);

    Message msg = {
        .type=MsgType::MSG_FILE_ID,
        .len=sizeof(payload)
    };
    memcpy(msg.payload, &payload, sizeof(payload));

    if (send(sock_fd, &msg, sizeof(msg), MSG_NOSIGNAL) == -1) {
        std::cout << "Cannot send data\n";
        return -1;
    }
    return 0;
}

int sendFile(int sock_fd, std::filesystem::path filePath) {
    std::ifstream fileToUpload(filePath, std::ifstream::binary);
    fileToUpload.seekg(0, fileToUpload.end);
    std::streampos fileSize = fileToUpload.tellg();
    fileToUpload.seekg(0, fileToUpload.beg);
    std::string filename = filePath.filename().string();

    unsigned int numBlocks = fileSize / MAX_PAYLOAD;
    if (fileSize % MAX_PAYLOAD > 0) {
        numBlocks++;
    }

    if (sendFileId(sock_fd, filename, numBlocks, fileSize) == -1) {
        return -1;
    }

    Message filePart = {.type=MsgType::MSG_FILEPART};
    bool success = true;
    for (int i = 0; i < numBlocks; i++) {
        fileToUpload.read(filePart.payload, MAX_PAYLOAD);
        filePart.len = fileToUpload.gcount();
        if (send(sock_fd, &filePart, sizeof(filePart), MSG_NOSIGNAL) == -1) {
            success = false;
            break;
        }
    }
    fileToUpload.close();
    
    if (!success)
        return -1;

    return waitForOk(sock_fd);
}
int receiveFileId(int sock_fd, FileId *fileId) {
    Message msg = readMessage(sock_fd);
    if (msg.type != MsgType::MSG_FILE_ID) 
        return -1;
    memcpy(fileId, msg.payload, msg.len);
    return 0;
}

int receiveFile(int sock_fd, std::filesystem::path filePath, int totalBlocks) {
    std::ofstream file(filePath, std::ios::binary);

    if (!file) {
        sendError(sock_fd, "Error while uploading file");
        return -1;
    }
    
    if (sendOk(sock_fd) == -1) {
        return -1;
    }
   
    for (int block = 0; block < totalBlocks; block++) {
        try {
            Message filePart = readMessage(sock_fd);
            file.write(filePart.payload, filePart.len);
        } catch (std::runtime_error) {
            sendError(sock_fd, "Invalid message");
            return -1;
        }    
    }

    if (sendOk(sock_fd) == -1)
        return -1;
    return 0;
}

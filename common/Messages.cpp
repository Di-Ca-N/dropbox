#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <cstring>
#include <iostream>
#include <ctime>

#include "Messages.hpp"

static std::map<MsgType, std::string> msgTypeNames = {
    {MsgType::MSG_AUTH, "MSG_AUTH"},
    {MsgType::MSG_SYNC_CLIENT_TO_SERVER, "MSG_SYNC_CLIENT_TO_SERVER"},
    {MsgType::MSG_SYNC_SERVER_TO_CLIENT, "MSG_SYNC_SERVER_TO_CLIENT"},
    {MsgType::MSG_OK, "MSG_OK"},
    {MsgType::MSG_ERROR, "MSG_ERROR"},
    {MsgType::MSG_UPLOAD, "MSG_UPLOAD"},
    {MsgType::MSG_DOWNLOAD, "MSG_DOWNLOAD"},
    {MsgType::MSG_LIST_SERVER, "MSG_LIST_SERVER"},
    {MsgType::MSG_FILE_ID, "MSG_FILE_ID"},
    {MsgType::MSG_FILEPART, "MSG_FILEPART"},
    {MsgType::MSG_FILE_OPERATION, "MSG_FILE_OPERATION"},
    {MsgType::MSG_NUM_FILES, "MSG_NUM_FILES"},
    {MsgType::MSG_FILE_METADATA, "MSG_FILE_METADATA"},
};

std::string toString(MsgType type) { return msgTypeNames[type]; }

void printMsg(Message &msg) {
    std::cout << "=== MESSAGE BEGIN ===\n";
    std::cout << "TYPE    : " << msgTypeNames[msg.type] << "\n";
    std::cout << "LEN     : " << msg.len << "\n";
    std::cout << "---- PAYLOAD ----\n";

    switch (msg.type) {
        case MsgType::MSG_FILE_ID: {
            FileId *payload = (FileId *)msg.payload;

            std::cout << "TOTAL BLOCKS : " << payload->totalBlocks << "\n";
            std::cout << "FILE SIZE    : " << payload->fileSize << "\n";
            std::cout << "FILENAME LEN : " << unsigned(payload->filenameSize)
                      << "\n";
            std::cout << "FILENAME     : "
                      << std::string(payload->filename,
                                     payload->filename + payload->filenameSize)
                      << "\n";
            break;
        }
        default:
            std::cout << "DATA : "
                      << std::string(msg.payload, msg.payload + msg.len)
                      << "\n";
            break;
    }

    std::cout << "=== MESSAGE END ===\n";
}

void printMeta(FileMeta &meta) {
    std::cout << meta.filename << std::endl;
    std::cout << "mtime: " << std::ctime(&(meta.mTime));
    std::cout << "atime: " << std::ctime(&(meta.aTime));
    std::cout << "ctime: " << std::ctime(&(meta.cTime));
}

u_int64_t getNumBlocks(u_int64_t fileSize, u_int64_t blockSize) {
    u_int64_t numBlocks = fileSize / blockSize;
    if (fileSize % blockSize != 0) {
        numBlocks++;
    }
    return numBlocks;
}


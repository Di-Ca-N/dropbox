#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "FileOp.hpp"
#include "Connection.hpp"
#include "Messages.hpp"
#include "utils.hpp"


void Connection::connectToServer(std::string username, std::string ip, int port) {
    int serverConnection = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    inet_aton(ip.data(), &addr.sin_addr);
 
    if (connect(serverConnection, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cout << "Error when connecting to server\n";
        return;
    }

    try {
        sendAuth(serverConnection, username);
        waitConfirmation(serverConnection);
        serverSock = serverConnection;
    } catch (BrokenPipe) {
        std::cout << "Connection closed during authentication\n";
        return;
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}

void Connection::upload(std::filesystem::path filepath) {
    if (serverSock == -1) {
        std::cout << "Server connection is closed\n";
        return;
    }

    std::ifstream file(filepath, std::ios::binary);

    if (!file) {
        std::cout << "Couldn't read file\n";
        return;
    }
    
    file.seekg(std::ios::end);
    u_int64_t fileSize = file.tellg();
    file.seekg(std::ios::beg);
    
    std::string filename = filepath.filename().string();

    FileId fid = {
        .totalBlocks = getNumBlocks(fileSize, MAX_PAYLOAD),
        .fileSize = fileSize,
        .filenameSize = static_cast<u_int8_t>(filename.length() + 1),
    };
    filename.copy(fid.filename, MAX_FILENAME);

    try {
        sendMessage(serverSock, MsgType::MSG_UPLOAD, nullptr, 0);
        waitConfirmation(serverSock);

        sendFileId(serverSock, fid);
        waitConfirmation(serverSock);

        sendFileData(serverSock, fid.totalBlocks, file);
        waitConfirmation(serverSock);

        std::cout << "Upload successful!\n";
    } catch (BrokenPipe) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }
}

void Connection::download(std::filesystem::path filepath) {
    std::string filename = filepath.filename().string();
    try {
        sendMessage(serverSock, MsgType::MSG_DOWNLOAD, nullptr, 0);
        waitConfirmation(serverSock);

        FileId fid = {.filenameSize = static_cast<u_int8_t>(filename.size() + 1)};
        filename.copy(fid.filename, MAX_FILENAME);

        sendFileId(serverSock, fid);
        waitConfirmation(serverSock);

        FileId fileData = receiveFileId(serverSock);
        sendOk(serverSock);

        std::ofstream file(filepath, std::ios::binary);
        receiveFileData(serverSock, fileData.totalBlocks, file);
        sendOk(serverSock);

        std::cout << "Download successful\n";
    } catch (BrokenPipe) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }
}

void Connection::delete_(std::filesystem::path filepath) {
    std::string filename = filepath.filename().string();

    FileId fid;
    filename.copy(fid.filename, MAX_FILENAME);
    fid.filenameSize = static_cast<u_int8_t>(filename.size() + 1);
    
    try {
        sendMessage(serverSock, MsgType::MSG_DELETE, nullptr, 0);
        waitConfirmation(serverSock);

        sendFileId(serverSock, fid);
        waitConfirmation(serverSock);

        std::cout << "File deleted successfully\n";
    } catch (BrokenPipe) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }
}

std::vector<FileMetadata> Connection::listServer() {
    // TODO
    return std::vector<FileMetadata>();
}

void Connection::syncWrite(FileOp op, std::string ogFilename, std::string newFilename) {
    switch (op) {
        case FileOp::OP_CHANGE:
            // TODO
            break;
        case FileOp::OP_DELETE:
            // TODO
            break;
        default:
            break;
    }
}

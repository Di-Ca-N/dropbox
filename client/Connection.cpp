#include <string>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "FileOp.hpp"
#include "Connection.hpp"
#include "BaseSocket.hpp"
#include "Messages.hpp"


void Connection::connectToServer(std::string username, std::string ip, int port) {
    try {
        serverSock = ClientSocket();
        serverSock.connect(ip, port);

        serverSock.sendAuth(username);
        serverSock.waitConfirmation();
    } catch (BrokenPipeException) {
        std::cout << "Connection closed during authentication\n";
        return;
    } catch (UnexpectedMsgTypeException) {
        std::cout << "Unexpected response.\n";
        return;
    } catch (ErrorReply &e) {
        std::cout << "Error: " << e.what() << "\n";
        return;
    }
}

void Connection::upload(std::filesystem::path filepath) {
    try {
        std::ifstream file(filepath, std::ios::binary);

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

        serverSock.sendMessage(MsgType::MSG_UPLOAD, nullptr, 0);
        serverSock.waitConfirmation();

        serverSock.sendFileId(fid);
        serverSock.waitConfirmation();

        serverSock.sendFileData(fid.totalBlocks, file);
        serverSock.waitConfirmation();

        std::cout << "Upload successful!" << std::endl;
    } catch (SocketIOError) {
        std::cout << "Server connection is closed" << std::endl;
    } catch (std::ifstream::failure) {
        std::cout << "Couldn't read file" << std::endl;
    } catch (BrokenPipeException) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply &e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgTypeException) {
        std::cout << "Unexpected response\n";
    }
}

void Connection::download(std::filesystem::path filepath) {
    std::string filename = filepath.filename().string();
    try {
        serverSock.sendMessage(MsgType::MSG_DOWNLOAD, nullptr, 0);
        serverSock.waitConfirmation();

        FileId fid = {.filenameSize = static_cast<u_int8_t>(filename.size() + 1)};
        filename.copy(fid.filename, MAX_FILENAME);

        serverSock.sendFileId(fid);
        serverSock.waitConfirmation();

        FileId fileData = serverSock.receiveFileId();
        serverSock.sendOk();

        std::ofstream file(filepath, std::ios::binary);
        serverSock.receiveFileData(fileData.totalBlocks, file);
        serverSock.sendOk();

        std::cout << "Download successful\n";
    } catch (BrokenPipeException) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply &e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgTypeException) {
        std::cout << "Unexpected response\n";
    }
}

void Connection::delete_(std::filesystem::path filepath) {
    std::string filename = filepath.filename().string();

    FileId fid;
    filename.copy(fid.filename, MAX_FILENAME);
    fid.filenameSize = static_cast<u_int8_t>(filename.size() + 1);
    
    try {
        serverSock.sendMessage(MsgType::MSG_DELETE, nullptr, 0);
        serverSock.waitConfirmation();

        serverSock.sendFileId(fid);
        serverSock.waitConfirmation();

        std::cout << "File deleted successfully\n";
    } catch (BrokenPipeException) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply &e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgTypeException) {
        std::cout << "Unexpected response\n";
    }
}

void Connection::listServer() {
    // TODO
}

void Connection::syncRead() {
    // TODO
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

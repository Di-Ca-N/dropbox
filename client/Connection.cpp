#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <poll.h>
#include <cstring>

#include "Connection.hpp"
#include "Messages.hpp"
#include "utils.hpp"


void Connection::connectToServer(std::string username, std::string ip, int port) {
    createSocket(commandSock, ip, port);
    authenticate(commandSock, username);
    
    createSocket(readSock, ip, port);
    authenticate(readSock, username);
    
    setReadConnection();
}

void Connection::createSocket(int &socketDescr, std::string ip, int port) {
    int serverConnection = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    inet_aton(ip.data(), &addr.sin_addr);
 
    if (connect(serverConnection, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cout << "Error when connecting to server\n";
        return;
    }

    socketDescr = serverConnection;
}

void Connection::authenticate(int &socketDescr, std::string username) {
    try {
        sendAuth(socketDescr, username);
        waitConfirmation(socketDescr);
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

void Connection::setReadConnection() {
    try {
        sendMessage(readSock, MsgType::MSG_SYNC_SERVER_TO_CLIENT, nullptr, 0);
        waitConfirmation(readSock);
    } catch (BrokenPipe) {
        std::cout << "Connection closed during authentication\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }
}

void Connection::upload(std::filesystem::path filepath) {
    if (commandSock == -1) {
        std::cout << "Server connection is closed\n";
        return;
    }

    std::ifstream file(filepath, std::ios::binary);

    if (!file) {
        std::cout << "Couldn't read file\n";
        return;
    }
    
    u_int64_t fileSize = std::filesystem::file_size(filepath);
    
    std::string filename = filepath.filename().string();

    FileId fid = {
        .totalBlocks = getNumBlocks(fileSize, MAX_PAYLOAD),
        .fileSize = fileSize,
        .filenameSize = static_cast<u_int8_t>(filename.length() + 1),
    };
    filename.copy(fid.filename, MAX_FILENAME);

    try {
        sendMessage(commandSock, MsgType::MSG_UPLOAD, nullptr, 0);
        waitConfirmation(commandSock);

        sendFileId(commandSock, fid);
        waitConfirmation(commandSock);

        sendFileData(commandSock, fid.totalBlocks, file);
        waitConfirmation(commandSock);

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
        sendMessage(commandSock, MsgType::MSG_DOWNLOAD, nullptr, 0);
        waitConfirmation(commandSock);

        FileId fid = {.filenameSize = static_cast<u_int8_t>(filename.size())};
        filename.copy(fid.filename, MAX_FILENAME);

        sendFileId(commandSock, fid);
        waitConfirmation(commandSock);

        FileId fileData = receiveFileId(commandSock);
        sendOk(commandSock);

        std::ofstream file(filepath, std::ios::binary);
        receiveFileData(commandSock, fileData.totalBlocks, file);
        sendOk(commandSock);

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
    fid.filenameSize = static_cast<u_int8_t>(filename.size());
    
    try {
        sendMessage(commandSock, MsgType::MSG_DELETE, nullptr, 0);
        waitConfirmation(commandSock);

        sendFileId(commandSock, fid);
        waitConfirmation(commandSock);

        std::cout << "File deleted successfully\n";
    } catch (BrokenPipe) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }
}

std::vector<FileMeta> Connection::listServer() {
    int numFiles;
    std::vector<FileMeta> fileMetas;

    try {
        sendMessage(commandSock, MsgType::MSG_LIST_SERVER, nullptr, 0);
        waitConfirmation(commandSock);

        numFiles = receiveNumFiles(commandSock);
        sendOk(commandSock);

        for (int i = 0; i < numFiles; i++)
            fileMetas.push_back(receiveFileMeta(commandSock));

        sendOk(commandSock);
    } catch(BrokenPipe) {
        std::cout << "Connection broken during upload\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }

    return fileMetas;
}


std::optional<FileOperation> Connection::syncRead() {
    int pollStatus;
    struct pollfd pfd;

    pfd.fd = readSock;
    pfd.events = POLLIN;

    pollStatus = poll(&pfd, 1, 200);

    if (pollStatus == -1) {
        std::cout << "Error while trying to poll server\n";
    } else if (pollStatus > 0) {
        return syncProcessRead();        
    }
    return std::nullopt;
}


std::optional<FileOperation> Connection::syncProcessRead() {
    FileId fileId;
    FileOpType fileOpType;

    try {
        fileOpType = receiveFileOperation(readSock);
        sendOk(readSock);

        fileId = receiveFileId(readSock);
        sendOk(readSock);
        switch (fileOpType) {
            case FileOpType::FILE_MODIFY:
                syncReadChange(fileId);
                break;
            case FileOpType::FILE_DELETE:
                syncReadDelete(fileId);
                break;
            default:
                break;
        }

        return makeFileOperation(fileId, fileOpType);
    } catch(BrokenPipe) {
        std::cout << "Connection broken during operation\n";
    } catch (ErrorReply e) {
        std::cout << "Error: " << e.what() << "\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }

    return std::nullopt;
}

FileOperation Connection::makeFileOperation(
        FileId &fileId,
        FileOpType &fileOpType) {
    FileOperation fileOp;

    fileOp.type = fileOpType;
    fileOp.filenameSize = fileId.filenameSize;
    strcpy(fileOp.filename, fileId.filename);

    return fileOp;
}

void Connection::syncReadChange(FileId &fileId) {
    std::ofstream stream;
    std::filesystem::path syncDir("sync_dir"); // FIXME: Pegar o nome desse diretório do jeito certo
    
    try {
        std::string filename(fileId.filename, fileId.filenameSize);
        std::filesystem::path filepath = syncDir / filename;
        stream.open(
                filepath,
                std::ofstream::binary
        );
        receiveFileData(readSock, fileId.totalBlocks, stream);
        stream.close();
        sendOk(readSock);
    } catch(BrokenPipe) {
        std::cout << "Connection broken during operation\n";
    } catch (UnexpectedMsgType) {
        std::cout << "Unexpected response\n";
    }
}

void Connection::syncReadDelete(FileId &fileId) {
    std::filesystem::path syncDir("sync_dir"); // FIXME: Pegar o nome desse diretório do jeito certo
    std::string filename(fileId.filename, fileId.filenameSize);
    std::filesystem::remove(syncDir / filename);
    sendOk(readSock);
}

void Connection::syncWrite(FileOpType op, std::filesystem::path target) {
    switch (op) {
        case FileOpType::FILE_MODIFY:
            // TODO
            break;
        case FileOpType::FILE_DELETE:
            // TODO
            break;
        default:
            break;
    }
}

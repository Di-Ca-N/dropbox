#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>

#include "Connection.hpp"
#include "Messages.hpp"
#include "utils.hpp"
#include "ClientConfig.hpp"


void Connection::connectToService(std::string username, std::string ip, int port) {
    this->username = username;

    in_addr_t binderIp = convertIpStringToNetwork(ip);
    in_port_t binderPort = convertPortIntToNetwork(port);

    connectToBinder(binderIp, binderPort);
    retryConnection();
}

in_addr_t Connection::convertIpStringToNetwork(std::string ipString) {
    in_addr ipNetwork;
    inet_aton(ipString.data(), &ipNetwork);
    return ipNetwork.s_addr;
}

in_port_t Connection::convertPortIntToNetwork(int port) {
    return htons(port);
}

void Connection::connectToBinder(in_addr_t &ip, in_port_t &port) {
    binderSock = createSocket(ip, port);
    if (binderSock == -1)
        throw BinderConnectionError();
}

void Connection::retryConnection() {
    ServiceStatus serviceStatus;
    ServerAddress serverAddress;

    sendMessage(binderSock, MsgType::MSG_STATUS_INQUIRY, nullptr, 0); 
    waitConfirmation(binderSock);

    serverAddress = receiveServerAddress(binderSock); 
    sendOk(binderSock);

    connectToServer(serverAddress.ip, serverAddress.port);
}

void Connection::connectToServer(in_addr_t &ip, in_port_t &port) {
    try {
        heartbeatSock = createSocket(ip, port);

        commandSock = createSocket(ip, port);
        authenticate(commandSock, username);

        readSock = createSocket(ip, port);
        authenticate(writeSock, username);
        setWriteConnection(writeSock);

        writeSock = createSocket(ip, port);
        authenticate(readSock, username);
        setReadConnection();
    } catch (const std::exception &e) {
        // Ou conecta todos, ou não conecta nenhum.
        close(heartbeatSock);
        close(commandSock);
        close(readSock);
        close(writeSock);

        throw e;
    }
}

bool Connection::hearsHeartbeat(int timeout) {
    try {
        waitHeartbeat(heartbeatSock, timeout);
    } catch (...) {
        return false;
    }

    return true;
}

int Connection::createSocket(in_addr_t &ip, in_port_t &port) {
    int serverConnection = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = port;
 
    if (connect(serverConnection, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cout << "Error when connecting to server\n";
        return -1;
    }

    return serverConnection;
}

void Connection::authenticate(int &socketDescr, std::string username) {
    AuthData authData = { .type=AuthType::AUTH_CLIENT };
    username.copy(authData.clientData.username, MAX_USERNAME);
    authData.clientData.usernameLen = username.length();
    if (this->deviceId == -1) {
        authData.clientData.deviceId = 0;
    } else {
        authData.clientData.deviceId = deviceId;
    }

    sendAuth(socketDescr, authData);
    AuthData authResponse = receiveAuth(socketDescr);

    if (this->deviceId == -1) {
        this->deviceId = authResponse.clientData.deviceId;
    }
}

void Connection::setWriteConnection(int &socketDescr) {
    sendMessage(socketDescr, MsgType::MSG_SYNC_CLIENT_TO_SERVER, nullptr, 0);
    waitConfirmation(socketDescr);
}

void Connection::setReadConnection() {
    sendMessage(readSock, MsgType::MSG_SYNC_SERVER_TO_CLIENT, nullptr, 0);
    waitConfirmation(readSock);
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
        .filenameSize = static_cast<u_int8_t>(filename.length()),
    };
    filename.copy(fid.filename, MAX_FILENAME);

    sendMessage(commandSock, MsgType::MSG_UPLOAD, nullptr, 0);
    waitConfirmation(commandSock);

    sendFileId(commandSock, fid);
    waitConfirmation(commandSock);

    sendFileData(commandSock, fid.totalBlocks, file);
    waitConfirmation(commandSock);
}

void Connection::download(std::filesystem::path filepath) {
    std::string filename = filepath.filename().string();

    sendMessage(commandSock, MsgType::MSG_DOWNLOAD, nullptr, 0);
    waitConfirmation(commandSock);

    FileId fid = {.filenameSize = static_cast<u_int8_t>(filename.size())};
    filename.copy(fid.filename, MAX_FILENAME);

    sendFileId(commandSock, fid);
    waitConfirmation(commandSock);

    FileId fileData = receiveFileId(commandSock);
    sendOk(commandSock);

    std::ofstream file(filepath, std::ios::binary);

    try {
        receiveFileData(commandSock, fileData.totalBlocks, file);
    } catch (const std::exception &e) {
        file.close();
        std::filesystem::remove(filepath);
        throw e;
    }

    sendOk(commandSock);
}

void Connection::delete_(std::filesystem::path filepath) {
    std::string filename = filepath.filename().string();

    FileId fid;
    filename.copy(fid.filename, MAX_FILENAME);
    fid.filenameSize = static_cast<u_int8_t>(filename.size());
    
    sendMessage(commandSock, MsgType::MSG_DELETE, nullptr, 0);
    waitConfirmation(commandSock);

    sendFileId(commandSock, fid);
    waitConfirmation(commandSock);
}

std::vector<FileMeta> Connection::listServer() {
    int numFiles;
    std::vector<FileMeta> fileMetas;

    sendMessage(commandSock, MsgType::MSG_LIST_SERVER, nullptr, 0);
    waitConfirmation(commandSock);

    numFiles = receiveNumFiles(commandSock);
    sendOk(commandSock);

    for (int i = 0; i < numFiles; i++)
        fileMetas.push_back(receiveFileMeta(commandSock));

    sendOk(commandSock);

    return fileMetas;
}


std::optional<FileOperation> Connection::syncRead() {
    int pollStatus;
    struct pollfd pfd;
    std::optional<FileOperation> operation;

    pfd.fd = readSock;
    pfd.events = POLLIN;

    pollStatus = poll(&pfd, 1, 1000);

    if (pollStatus == -1) {
        std::cout << "Error while trying to poll server\n";
    } else if (pollStatus > 0) {
        operation = syncProcessRead();
    }

    return operation;
}

std::optional<FileOperation> Connection::syncProcessRead() {
    FileId fileId;
    FileOpType fileOpType;

    fileOpType = receiveFileOperation(readSock);
    sendOk(readSock);

    if (fileOpType == FileOpType::VOID_OP) {
        sendOk(readSock);
        return std::nullopt;
    }

    fileId = receiveFileId(readSock);
    sendOk(readSock);

    FileOperation fileOperation = makeFileOperation(fileId, fileOpType);

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

    return fileOperation;
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
    std::filesystem::path syncDir(SYNC_DIR);

    std::string filename(fileId.filename, fileId.filenameSize);
    std::filesystem::path filepath = syncDir / filename;
    filepath.concat(TEMP_FILE_EXT); // Do not change file directly. Let monitor do that.
    stream.open(
            filepath,
            std::ofstream::binary
    );

    try {
        receiveFileData(readSock, fileId.totalBlocks, stream);
    } catch (const std::exception &e) {
        stream.close();
        std::filesystem::remove(filepath);
        throw e;
    }

    stream.close();
    sendOk(readSock);
}

void Connection::syncReadDelete(FileId &fileId) {
    std::filesystem::path syncDir(SYNC_DIR);
    std::string filename(fileId.filename, fileId.filenameSize);
    std::filesystem::remove(syncDir / filename);
    sendOk(readSock);
}

void Connection::syncWrite(FileOpType op, std::filesystem::path target) {
    switch (op) {
        case FileOpType::FILE_MODIFY:
            sendChange(target);
            break;
        case FileOpType::FILE_DELETE:
            sendDelete(target);
            break;
        default:
            break;
    }
}
void Connection::sendChange(std::filesystem::path target) {
    std::filesystem::path syncDir(SYNC_DIR);
    std::filesystem::path filepath = syncDir / target;

    std::ifstream file;
    FileId fileId;
    try {
        fileId = getFileId(filepath);
    } catch (std::filesystem::filesystem_error) {
        // This happens if the file was deleted before we could send it's changes to the server.
        // It is a common problem with temporary files created by some programs (such as vim or gedit)
        return;
    }

    sendFileOperation(writeSock, FileOpType::FILE_MODIFY);
    waitConfirmation(writeSock);

    sendFileId(writeSock, fileId);
    waitConfirmation(writeSock);

    file = std::ifstream(filepath, std::ios::binary);
    sendFileData(writeSock, fileId.totalBlocks, file);
    waitConfirmation(writeSock);
}

void Connection::sendDelete(std::filesystem::path target) {
    FileId fileId;
    std::string filename = target.filename().string();
    filename.copy(fileId.filename, MAX_FILENAME);
    fileId.filenameSize = filename.size();

    sendFileOperation(writeSock, FileOpType::FILE_DELETE);
    waitConfirmation(writeSock);

    sendFileId(writeSock, fileId);
    waitConfirmation(writeSock);
}

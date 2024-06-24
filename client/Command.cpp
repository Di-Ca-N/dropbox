#include <iostream>
#include <string.h>
#include <sys/stat.h>

#include "Command.hpp"

void printMeta(FileMeta &meta) {
    std::cout <<  meta.filename<< std::endl;
    std::cout << "mtime: " << std::ctime(&meta.mTime);
    std::cout << "atime: " << std::ctime(&meta.aTime);
    std::cout << "ctime: " << std::ctime(&meta.cTime);
    std::cout << std::endl;
}

Upload::Upload(std::weak_ptr<Connection> connection,
        std::filesystem::path target) {
    this->connection = connection;
    this->target = target;
}

void Upload::execute() {
    std::shared_ptr<Connection> sharedPtr;
    if ((sharedPtr = connection.lock()))
        sharedPtr->upload(target);
}

Download::Download(std::weak_ptr<Connection> connection,
        std::filesystem::path target) {
    this->connection = connection;
    this->target = target;
}

void Download::execute() {
    std::shared_ptr<Connection> sharedPtr;
    if ((sharedPtr = connection.lock()))
        sharedPtr->download(target);
}

Delete::Delete(std::weak_ptr<Connection> connection,
        std::filesystem::path target) {
    this->connection = connection;
    this->target = target;
}

void Delete::execute() {
    std::shared_ptr<Connection> sharedPtr;
    if ((sharedPtr = connection.lock()))
        sharedPtr->delete_(target);
}

ListServer::ListServer(std::weak_ptr<Connection> connection) {
    this->connection = connection;
}

void ListServer::execute() {
    std::shared_ptr<Connection> sharedPtr;
    if ((sharedPtr = connection.lock())) {
        for (auto meta : sharedPtr->listServer())
            printMeta(meta);
    }
}

ListClient::ListClient(std::filesystem::path syncDirPath) {
    this->syncDirPath = syncDirPath;
}

void ListClient::execute() {
    struct stat fileStat;
    FileMeta meta;

    for (const auto& file : std::filesystem::directory_iterator(syncDirPath)) {
        if (stat(file.path().c_str(), &fileStat) == 0) {
            strncpy(meta.filename, file.path().c_str(), MAX_FILENAME); 
            meta.mTime = fileStat.st_mtime;
            meta.aTime = fileStat.st_atime;
            meta.cTime = fileStat.st_ctime;
            printMeta(meta);
        }
    }
}

GetSyncDir::GetSyncDir(std::filesystem::path syncDirPath,
        std::weak_ptr<ThreadOwner> threadOwner,
        std::weak_ptr<ClientState> clientState,
        std::weak_ptr<Connection> connection) {
    this->syncDirPath = syncDirPath;
    this->threadOwner = threadOwner;
    this->clientState = clientState;
    this->connection = connection;
}

void GetSyncDir::execute() {
    std::shared_ptr<ThreadOwner> sharedOwner;
    std::shared_ptr<ClientState> sharedState;
    std::shared_ptr<Connection> sharedConnection;

    if (!std::filesystem::exists(syncDirPath)
            || !std::filesystem::is_directory(syncDirPath))
        std::filesystem::create_directory(syncDirPath);
    
    if ((sharedOwner = threadOwner.lock()) &&
            (sharedState = clientState.lock()) &&
            (sharedConnection = connection.lock())) {
        if (sharedState->get() == AppState::STATE_UNTRACKED) {
            sharedOwner->restartServerThread();
            sharedOwner->restartClientThread();
        }

        sharedState->setActiveIfNotClosing();
    }
}

Exit::Exit(std::weak_ptr<ClientState> clientState) {
    this->clientState = clientState;
}

void Exit::execute() {
    std::shared_ptr<ClientState> sharedPtr;
    if ((sharedPtr = clientState.lock()))
        sharedPtr->setClosing();
}

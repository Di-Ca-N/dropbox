#ifndef COMMAND_H
#define COMMAND_H

#include <filesystem>

#include "ThreadOwner.hpp"
#include "Connection.hpp"
#include "ClientState.hpp"

class Command {
public:
    virtual void execute() = 0;
};

class Upload : public Command {
    std::weak_ptr<Connection> connection;
    std::filesystem::path target;
public:
    Upload(std::weak_ptr<Connection> connection,
            std::filesystem::path target);
    void execute() override;
};

class Download : public Command {
    std::weak_ptr<Connection> connection;
    std::filesystem::path target;
public:
    Download(std::weak_ptr<Connection> connection,
            std::filesystem::path target);
    void execute() override;
};

class Delete : public Command {
    std::weak_ptr<Connection> connection;
    std::filesystem::path target;
public:
    Delete(std::weak_ptr<Connection> connection,
            std::filesystem::path target);
    void execute() override;
};

class ListServer : public Command {
    std::weak_ptr<Connection> connection;
public:
    ListServer(std::weak_ptr<Connection> connection);
    void execute() override;
};

class ListClient : public Command {
    std::filesystem::path syncDirPath;
public:
    ListClient(std::filesystem::path syncDirPath);
    void execute() override;
};

class GetSyncDir : public Command {
    std::filesystem::path syncDirPath;
    std::weak_ptr<ThreadOwner> threadOwner;
    std::weak_ptr<ClientState> clientState;
    std::weak_ptr<Connection> connection;
public:
    GetSyncDir(std::filesystem::path syncDirPath,
            std::weak_ptr<ThreadOwner> threadOwner,
            std::weak_ptr<ClientState> clientState,
            std::weak_ptr<Connection> connection);
    void execute() override;
};

class Exit : public Command {
    std::weak_ptr<ClientState> clientState;
public:
    Exit(std::weak_ptr<ClientState> clientState);
    void execute() override;
};

#endif

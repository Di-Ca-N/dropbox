#ifndef COMMAND_CREATOR_H
#define COMMAND_CREATOR_H

#include <vector>

#include "Command.hpp"

class CommandCreator {
public:
    virtual std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) = 0;
};

class InvalidArgumentException : public std::runtime_error {
public:
    InvalidArgumentException(const std::string& message)
        : std::runtime_error(message) {}
};

class UploadCreator : public CommandCreator {
    std::weak_ptr<Connection> connection;

public:
    UploadCreator(
            std::weak_ptr<Connection> connection
    );
    std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) override;
};

class DownloadCreator : public CommandCreator {
    std::weak_ptr<Connection> connection;

public:
    DownloadCreator(
            std::weak_ptr<Connection> connection
    );
    std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) override;
};

class DeleteCreator : public CommandCreator {
    std::weak_ptr<Connection> connection;

public:
    DeleteCreator(
            std::weak_ptr<Connection> connection
    );
    std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) override;
};

class ListServerCreator : public CommandCreator {
    std::weak_ptr<Connection> connection;

public:
    ListServerCreator(std::weak_ptr<Connection> connection);
    std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) override;
};

class ListClientCreator : public CommandCreator {
    std::filesystem::path syncDirPath;

public:
    ListClientCreator(std::filesystem::path syncDirPath);
    std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) override;
};

class GetSyncDirCreator : public CommandCreator {
    std::filesystem::path syncDirPath;
    std::weak_ptr<ThreadOwner> threadOwner;
    std::weak_ptr<ClientState> clientState;
    std::weak_ptr<Connection> connection;

public:
    GetSyncDirCreator(
            std::filesystem::path syncDirPath,
            std::weak_ptr<ThreadOwner> threadOwner,
            std::weak_ptr<ClientState> clientState,
            std::weak_ptr<Connection> connection
    );
    std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) override;
};

class ExitCreator : public CommandCreator {
    std::weak_ptr<ClientState> clientState;

public:
    ExitCreator(std::weak_ptr<ClientState> clientState);
    std::unique_ptr<Command> create(
            std::vector<std::string> &tokens
    ) override;
};

#endif

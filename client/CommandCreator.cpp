#include "CommandCreator.hpp"

UploadCreator::UploadCreator(
        std::weak_ptr<Connection> connection) {
    this->connection = connection;
}

std::unique_ptr<Command> UploadCreator::create(
        std::vector<std::string> &tokens) {
    if (tokens.size() != 2)
        throw InvalidArgumentException("Format: upload <path/filename.ext>");

    return std::make_unique<Upload>(Upload(connection, tokens[1]));
}

DownloadCreator::DownloadCreator(
        std::weak_ptr<Connection> connection) {
    this->connection = connection;
}

std::unique_ptr<Command> DownloadCreator::create(
        std::vector<std::string> &tokens) {
    if (tokens.size() != 2)
        throw InvalidArgumentException("Format: download <filename.ext>");

    return std::make_unique<Download>(Download(connection, tokens[1]));
}

DeleteCreator::DeleteCreator(
        std::weak_ptr<Connection> connection) {
    this->connection = connection;
}

std::unique_ptr<Command> DeleteCreator::create(
        std::vector<std::string> &tokens) {
    if (tokens.size() != 2)
        throw InvalidArgumentException("Format: delete <filename.ext>");

    return std::make_unique<Delete>(Delete(connection, tokens[1]));
}

ListServerCreator::ListServerCreator(std::weak_ptr<Connection> connection) {
    this->connection = connection;
}

std::unique_ptr<Command> ListServerCreator::create(
        std::vector<std::string> &tokens) {
    if (tokens.size() != 1)
        throw InvalidArgumentException("Format: list_server");

    return std::make_unique<ListServer>(ListServer(connection)); 
}

ListClientCreator::ListClientCreator(std::filesystem::path syncDirPath) {
    this->syncDirPath = syncDirPath;
}

std::unique_ptr<Command> ListClientCreator::create(
        std::vector<std::string> &tokens) {
    if (tokens.size() != 1)
        throw InvalidArgumentException("Format: list_client");

    return std::make_unique<ListClient>(ListClient(syncDirPath));
}

GetSyncDirCreator::GetSyncDirCreator(
        std::filesystem::path syncDirPath,
        std::weak_ptr<ThreadOwner> threadOwner,
        std::weak_ptr<ClientState> clientState,
        std::weak_ptr<Connection> connection) {
    this->syncDirPath = syncDirPath;
    this->threadOwner = threadOwner;
    this->clientState = clientState;
    this->connection = connection;
}

std::unique_ptr<Command> GetSyncDirCreator::create(
        std::vector<std::string> &tokens) {
    if (tokens.size() != 1)
        throw InvalidArgumentException("Format: get_sync_dir");

    return std::make_unique<GetSyncDir>(
            GetSyncDir(
                syncDirPath,
                threadOwner,
                clientState,
                connection)
    );
}

ExitCreator::ExitCreator(std::weak_ptr<ClientState> clientState) {
    this->clientState = clientState;
}

std::unique_ptr<Command> ExitCreator::create(
        std::vector<std::string> &tokens) {
    if (tokens.size() != 1)
        throw InvalidArgumentException("Format: exit");

    return std::make_unique<Exit>(Exit(clientState));
}

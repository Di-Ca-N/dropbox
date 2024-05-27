#include <string>
#include <vector>
#include <unordered_map>

#include "CommandParser.hpp"

CommandParser::CommandParser(
        std::filesystem::path syncDirPath,
        std::weak_ptr<ThreadOwner> threadOwner,
        std::weak_ptr<ClientState> clientState,
        std::weak_ptr<Connection> connection) {
    commandMap = {
        {"upload", std::make_shared<UploadCreator>(connection)},
        {"download", std::make_shared<DownloadCreator>(connection)},
        {"delete", std::make_shared<DeleteCreator>(connection)},
        {"list_server", std::make_shared<ListServerCreator>(connection)},
        {"list_client", std::make_shared<ListClientCreator>(syncDirPath)},
        {"get_sync_dir", std::make_shared<GetSyncDirCreator>(
                                                syncDirPath,
                                                threadOwner,
                                                clientState,
                                                connection)},
        {"exit", std::make_shared<ExitCreator>(clientState)}
    };
}

std::unique_ptr<Command> CommandParser::parse(std::string &command) {
    std::vector<std::string> tokens;

    getTokens(tokens, command);

    return createCommand(tokens);
}

void CommandParser::getTokens(
        std::vector<std::string> &tokens,
        std::string command) {
    std::string token;
    std::istringstream iss;

    iss = std::istringstream(command);
    while (iss >> token)
        tokens.push_back(token);
}

std::unique_ptr<Command> CommandParser::createCommand(
        std::vector<std::string> &tokens) {
    if (tokens.size() < 1)
        throw NoCommandException("No command informed");

    auto it = commandMap.find(tokens[0]);

    if (it == commandMap.end())
        throw InvalidCommandException("The command doesn't exist");

    std::shared_ptr<CommandCreator> creator = it->second;

    return creator->create(tokens);
}

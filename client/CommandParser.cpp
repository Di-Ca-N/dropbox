#include <string>
#include <vector>
#include <unordered_map>

#include "CommandParser.hpp"

enum class CommandString {
    COMMAND_UPLOAD,
    COMMAND_DOWNLOAD,
    COMMAND_DELETE,
    COMMAND_LIST_SERVER,
    COMMAND_LIST_CLIENT,
    COMMAND_GET_SYNC_DIR,
    COMMAND_EXIT
};

static std::unordered_map<std::string, CommandString> commandMap {
    {"upload", CommandString::COMMAND_UPLOAD},
    {"download", CommandString::COMMAND_DOWNLOAD},
    {"delete", CommandString::COMMAND_DELETE},
    {"list_server", CommandString::COMMAND_LIST_SERVER},
    {"list_client", CommandString::COMMAND_LIST_CLIENT},
    {"get_sync_dir", CommandString::COMMAND_GET_SYNC_DIR},
    {"exit", CommandString::COMMAND_EXIT},
};

std::unique_ptr<Command> CommandParser::parse(std::string command,
        std::filesystem::path syncDirPath,
        std::weak_ptr<ThreadOwner> threadOwner,
        std::weak_ptr<ClientState> clientState,
        std::weak_ptr<Connection> connection) {
    std::string token;
    std::istringstream iss;
    std::vector<std::string> tokens;
    std::unique_ptr<Command> cmd;

    iss = std::istringstream(command);
    while (iss >> token)
        tokens.push_back(token);

    if (tokens.size() < 1)
        throw NoCommandException("No command informed");

    auto it = commandMap.find(tokens[0]);

    if (it == commandMap.end())
        throw InvalidCommandException("The command doesn't exist");

    switch (it->second) {
        case CommandString::COMMAND_UPLOAD:
            if (tokens.size() != 2)
                throw InvalidArgumentException("Wrong command format");
            if (std::filesystem::exists(tokens[1]))
                cmd = std::make_unique<Upload>(Upload(connection, tokens[1]));
            else
                throw InvalidArgumentException("Path is invalid or doesn't exist");
            break;
        case CommandString::COMMAND_DOWNLOAD:
            if (tokens.size() != 2)
                throw InvalidArgumentException("Wrong command format");
            cmd = std::make_unique<Download>(Download(connection, tokens[1]));
            break;
        case CommandString::COMMAND_DELETE:
            if (tokens.size() != 2)
                throw InvalidArgumentException("Wrong command format");
            cmd = std::make_unique<Delete>(Delete(connection, tokens[1]));
            break;
        case CommandString::COMMAND_LIST_SERVER:
            if (tokens.size() != 1)
                throw InvalidArgumentException("Wrong command format");
            cmd = std::make_unique<ListServer>(ListServer(connection)); 
            break;
        case CommandString::COMMAND_LIST_CLIENT:
            if (tokens.size() != 1)
                throw InvalidArgumentException("Wrong command format");
            cmd = std::make_unique<ListClient>(ListClient(syncDirPath));
            break;
        case CommandString::COMMAND_GET_SYNC_DIR:
            if (tokens.size() != 1)
                throw InvalidArgumentException("Wrong command format");
            cmd = std::make_unique<GetSyncDir>(
                    GetSyncDir(syncDirPath,
                        threadOwner,
                        clientState,
                        connection)
            );
            break;
        case CommandString::COMMAND_EXIT:
            if (tokens.size() != 1)
                throw InvalidArgumentException("Wrong command format");
            cmd = std::make_unique<Exit>(Exit(clientState));
            break;
    }

    return std::move(cmd);
}

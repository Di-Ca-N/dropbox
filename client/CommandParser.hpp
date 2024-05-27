#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "Command.hpp"
#include "CommandCreator.hpp"

class CommandParser {
    std::unordered_map<std::string, std::shared_ptr<CommandCreator>> commandMap;

    void getTokens(
            std::vector<std::string> &tokens,
            std::string command);

    std::unique_ptr<Command> createCommand(std::vector<std::string> &tokens);

public:
    CommandParser(
            std::filesystem::path syncDirPath,
            std::weak_ptr<ThreadOwner> threadOwner,
            std::weak_ptr<ClientState> clientState,
            std::weak_ptr<Connection> connection
    );
    std::unique_ptr<Command> parse(std::string &command);
};

class NoCommandException : public std::runtime_error {
public:
    NoCommandException(const std::string &message)
        : std::runtime_error(message) {}
};

class InvalidCommandException : public std::runtime_error {
public:
    InvalidCommandException(const std::string &message)
        : std::runtime_error(message) {}
};

#endif

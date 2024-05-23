#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdexcept>
#include <string>

#include "Command.hpp"

class CommandParser {
public:
    static std::unique_ptr<Command> parse(std::string command,
            std::filesystem::path syncDirPath,
            std::weak_ptr<ThreadOwner> threadOwner,
            std::weak_ptr<ClientState> clientState,
            std::weak_ptr<Connection> connection);
};

class NoCommandException : public std::runtime_error {
public:
    NoCommandException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidCommandException : public std::runtime_error {
public:
    InvalidCommandException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidArgumentException : public std::runtime_error {
public:
    InvalidArgumentException(const std::string& message)
        : std::runtime_error(message) {}
};

#endif

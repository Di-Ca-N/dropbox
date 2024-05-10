#include <string>
#include <vector>

#include "FileOp.hpp"
#include "Connection.hpp"

void Connection::connectToServer(std::string username, std::string ip, int port) {
    // TODO
}

void Connection::upload(std::string filepath) {
    // TODO
}

void Connection::download(std::string filepath) {
    // TODO
}

void Connection::delete_(std::string filepath) {
    // TODO
}

std::vector<FileMetadata> Connection::listServer() {
    // TODO
    return std::vector<FileMetadata>();
}

void Connection::syncRead() {
    // TODO
}

void Connection::syncWrite(FileOp op, std::string ogFilename, std::string newFilename) {
    switch (op) {
        case FileOp::OP_CHANGE:
            // TODO
            break;
        case FileOp::OP_DELETE:
            // TODO
            break;
        default:
            break;
    }
}

#include <memory>
#include <iostream>
#include <filesystem>
#include <vector>
#include <sys/stat.h>
#include <poll.h>
#include <unistd.h>

#include "CLI.hpp"
#include "ClientState.hpp"
#include "ServerMonitor.hpp"
#include "ClientMonitor.hpp"
#include "FileMetadata.hpp"

#define SYNC_DIR "sync_dir"

void CLI::run(std::string username, std::string ip, int port) {
    connection = std::make_shared<Connection>(Connection());
    connection->connectToServer(username, ip, port);

    clientState = std::make_shared<ClientState>(AppState::STATE_UNTRACKED);

    getSyncDir();

    bool nextLine = true;

    struct pollfd cinFd = { .fd = STDIN_FILENO, .events = POLLIN };

    std::string line;
    std::string token;
    std::istringstream iss;
    std::vector<std::string> tokens;
    while (clientState->get() != AppState::STATE_CLOSING) {
        if (nextLine) {
            std::cout << "> ";
            std::cout.flush();
            nextLine = false;
        }

        int cinPoll = poll(&cinFd, 1, 0);

        if (cinPoll > 0 && cinFd.revents & POLLIN) {
            std::getline(std::cin, line);
            iss = std::istringstream(line);
            tokens.clear();
            while (iss >> token)
                tokens.push_back(token);
            nextLine = true;

            if (tokens.size() == 1 && tokens[0].compare("exit") == 0)
                clientState->setClosing();
            else if (tokens.size() == 1 && tokens[0].compare("get_sync_dir") == 0)
                getSyncDir();
            else if (tokens.size() == 2 && tokens[0].compare("upload") == 0)
                connection->upload(tokens[1]);
            else if (tokens.size() == 2 && tokens[0].compare("download") == 0)
                connection->download(tokens[1]);
            else if (tokens.size() == 2 && tokens[0].compare("delete") == 0)
                connection->delete_(tokens[1]);
            else if (tokens.size() == 1 && tokens[0].compare("list_client") == 0)
                listClient();
            else if (tokens.size() == 1 && tokens[0].compare("list_server") == 0)
                for (auto meta : connection->listServer())
                    printFileMetadata(meta);
            else
                std::cerr << "Command not recognized" << std::endl;
        }
    }

    serverThread.join();
    clientThread.join();
}

void CLI::getSyncDir() {
    if (!std::filesystem::exists(SYNC_DIR)
            || !std::filesystem::is_directory(SYNC_DIR))
        std::filesystem::create_directory(SYNC_DIR);

    if (clientState->get() == AppState::STATE_UNTRACKED) {
        if (serverThread.joinable())
            serverThread.join();

        if (clientThread.joinable())
            clientThread.join();

        serverMonitor = std::make_unique<ServerMonitor>(ServerMonitor(
                                                           clientState, connection));
        serverThread = std::thread(&ServerMonitor::run, std::ref(*serverMonitor));
        clientMonitor = std::make_unique<ClientMonitor>(ClientMonitor(
                                                           clientState, connection));
        clientThread = std::thread(&ClientMonitor::run, std::ref(*clientMonitor), SYNC_DIR);
    }

    clientState->setActiveIfNotClosing();
}

void CLI::listClient() {
    struct stat fileStat;
    FileMetadata meta;

    for (const auto& file : std::filesystem::directory_iterator(SYNC_DIR)) {
        if (stat(file.path().c_str(), &fileStat) == 0) {
            meta.filepath = file.path().string();
            meta.mtime = fileStat.st_mtime;
            meta.atime = fileStat.st_atime;
            meta.ctime = fileStat.st_ctime;
            printFileMetadata(meta);
        }
    }
}

void CLI::printFileMetadata(FileMetadata& fileMeta) {
    std::cout << fileMeta.filepath << std::endl;
    std::cout << "mtime: " << std::ctime(&fileMeta.mtime);
    std::cout << "atime: " << std::ctime(&fileMeta.atime);
    std::cout << "ctime: " << std::ctime(&fileMeta.ctime);
    std::cout << std::endl;
}

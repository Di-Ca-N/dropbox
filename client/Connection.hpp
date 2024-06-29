#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

#include "Messages.hpp"

class Connection {
private:
<<<<<<< Updated upstream
=======
<<<<<<< Updated upstream
    int serverSock = -1;
=======
>>>>>>> Stashed changes
    int commandSock = -1;
    int readSock = -1;

    void createSocket(int &socketDescr, std::string ip, int port);
    void authenticate(int &socketDescr, std::string username);
    void setReadConnection();
<<<<<<< Updated upstream
    void syncProcessRead();
    void syncReadChange(FileId &fileId);
    void syncReadDelete(FileId &fileId);
=======
    std::optional<FileOperation> syncProcessRead();
    void syncReadChange(FileId &fileId);
    void syncReadDelete(FileId &fileId);
    FileOperation makeFileOperation(FileId &fileId, FileOpType &fileOpType);
>>>>>>> Stashed changes
>>>>>>> Stashed changes

public:
    void connectToServer(std::string username, std::string ip, int port);
    void upload(std::filesystem::path filepath);
    void download(std::filesystem::path filepath);
    void delete_(std::filesystem::path filepath);
    std::vector<FileMeta> listServer();
<<<<<<< Updated upstream
    void syncRead();
<<<<<<< Updated upstream
    void syncWrite(FileOpType op, std::filesystem::path target);
=======
    void syncWrite(FileOp op, std::string ogFilename, std::string newFilename);
=======
    std::optional<FileOperation> syncRead();
    void syncWrite(FileOpType op, std::filesystem::path target);
>>>>>>> Stashed changes
>>>>>>> Stashed changes
};

#endif

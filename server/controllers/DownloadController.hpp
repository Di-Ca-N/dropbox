#include "Controller.hpp"

#include <string>

class DownloadController : public Controller {
    private:
        int sock_fd;
        std::string username;

    public:
        DownloadController(int sock_fd, std::string username);
        void run();
};
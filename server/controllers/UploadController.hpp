#include "Controller.hpp"

#include <string>

class UploadController : public Controller {
    private:
        int sock_fd;
        std::string username;

    public:
        UploadController(int sock_fd, std::string username);
        void run();
};
#include <map>
#include <string>
#include <thread>

#include "DeviceManager.hpp"

class Server {
  private:
    int port;
    int sock_fd;
    bool running = false;
    std::vector<std::thread> openConnections;
    std::map<std::string, DeviceManager*> deviceManagers;

  public:
    Server(int port);
    ~Server();
    void run();
    void handleClient(int client_socket);
};

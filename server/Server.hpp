#include <map>
#include <string>
#include <thread>

#include "DeviceManager.hpp"
#include "controllers/Controller.hpp"

// class ClientHandler {
//   private:
//     bool running = false;
//     int client_sock;
//     Controller *controller;

//   public:
//     ClientHandler(int client_sock);
//     ~ClientHandler();
//     void run();
//     void stop();
// };

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
    void stop();
    void run();
};

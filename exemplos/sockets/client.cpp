#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <string>

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    inet_aton("127.0.0.1", &addr.sin_addr);

    int server = connect(sock_fd, (sockaddr*) &addr, sizeof(addr));

    if (server == -1) {
        std::cout << "Erro\n";
        return 1; 
    }
    std::string msg;
    while (true) {
        std::cin >> msg;

        int sent = send(sock_fd, msg.data(), msg.size() + 1, 0);

        std::cout << "Enviou " << sent << " bytes\n";
    }

    return 0;
}
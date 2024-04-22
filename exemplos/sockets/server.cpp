#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_DGRAM
    int V = 1;
    setsockopt(sock_fd, 1, SO_REUSEADDR, &V, sizeof(V));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 8000;
    inet_aton("127.0.0.1", &addr.sin_addr);

    bind(sock_fd, (sockaddr*) &addr, sizeof(addr)); // Liga o socket no endereço 127.0.0.1:8000

    listen(sock_fd, 10); // Socket começa a escutar na porta

    int client = accept(sock_fd, nullptr, nullptr);
    std::cout << "Cliente conectou\n";
    char buffer[256];
    while (true) {
        int bytes = recv(client, buffer, 256, 0);

        if (bytes == 0) {
            break;
        }
        std::cout << buffer << std::endl;
    }

    close(client);
    close(sock_fd);

    return 0;
}
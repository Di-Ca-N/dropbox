#include <sys/inotify.h>
#include <sys/epoll.h>
#include <vector>
#include <queue>
#include <iostream>
#include <sys/unistd.h>

#define INOTIFY_EVENT_MAX_SIZE (sizeof(inotify_event) + 256)
#define MAX_INOTIFY_EVENTS 10

int main() {
    // Inicializa o inotify
    int inotify_fd = inotify_init();

    // Define o diretório e quais eventos serão observados
    inotify_add_watch(inotify_fd, "sync_dir", IN_DELETE | IN_CLOSE_WRITE | IN_MOVE | IN_CREATE);
    
    // Cria uma instância epoll, para ficar observando o FD do inotify por eventos
    int epoll_fd = epoll_create(1);

    // Adiciona o FD do inotify na instância do epoll
    epoll_event settings;
    settings.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &settings);

    // Array que guarda os eventos epoll (isto é, eventos disparados por modificações no fd do inotify)
    // Nesse caso vamos tratar só com um evento por vez
    epoll_event events[1];

    // Buffer para guardar os eventos do inotify. Guarda até 10 eventos 
    char buffer[INOTIFY_EVENT_MAX_SIZE * MAX_INOTIFY_EVENTS];

    while(true) {
        // Espera por um evento epoll, ou seja, uma modificação no FD do inotify.
        // O -1 no final indica que deve esperar para sempre
        int numEvents = epoll_wait(epoll_fd, events, 1, -1);

        std::cout << "Registrados " << numEvents << " eventos epoll\n";

        for (int i = 0; i < numEvents; i++) {
            // Lê o fd do inotify
            int readBytes = read(inotify_fd, buffer, sizeof(buffer));
            std::cout << "Read " << readBytes << " bytes\n";

            // Percorre os eventos. É necessário fazer esse tratamento mais complexo ao invés de simplesmente
            // usar um array porque os eventos têm tamanho variável.

            char *ptr = buffer;

            while (ptr < buffer + readBytes) {
                inotify_event *event = (inotify_event*) ptr;

                // Usa a máscara para diferenciar cada tipo de evento
                if (event->mask & IN_DELETE) {
                    std::cout << "File '" << event->name << "' was deleted." << std::endl;    
                } else if (event->mask & IN_CREATE) {
                    std::cout << "File '" << event->name << "' was created." << std::endl;
                } else if (event->mask & IN_MOVED_FROM) {
                    std::cout << "File '" << event->name << "' was moved from folder." << std::endl;
                } else if (event->mask & IN_MOVED_TO) {
                    std::cout << "File '" << event->name << "' was moved to folder." << std::endl;
                } else if (event->mask & IN_CLOSE_WRITE) {
                    std::cout << "File '" << event->name << "' was modified." << std::endl;
                }
                ptr += sizeof(inotify_event) + event->len;
            }
        }
    }

    // Fecha os descritores de arquivo
    close(epoll_fd);
    close(inotify_fd);
    return 0;
}
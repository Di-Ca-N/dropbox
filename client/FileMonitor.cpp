#include <sys/inotify.h>
#include <sys/epoll.h>
#include <vector>
#include <queue>
#include <iostream>
#include <sys/unistd.h>

int main() {
    int inotify = inotify_init();

    inotify_add_watch(inotify, "sync_dir", IN_MODIFY | IN_DELETE);
    
    int epoll = epoll_create(1);
    epoll_event settings;
    settings.events = EPOLLIN;
    epoll_ctl(epoll, EPOLL_CTL_ADD, inotify, &settings);

    epoll_event events[1];

    while(true) {
        int n_events = epoll_wait(epoll, events, 1, -1);

        std::cout << "Registrados " << n_events << " eventos epoll\n";

        inotify_event buffer[20];
        for (int i = 0; i < n_events; i++) {
            int readBytes = read(inotify, buffer, sizeof(buffer));
            int n = readBytes / sizeof(inotify_event);
            std::cout << "Registrados " << n << " eventos inotify\n";
            for (int j = 0; j < n; j++) {
                std::cout << "File '" << buffer[j].name << "' was modified or moved." << std::endl;
            }
                      
        }
    }
    return 0;
}
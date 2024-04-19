#include <stdio.h>
#include <sys/socket.h>
#include <innet/in.h>
#include <innet/tcp.h>

int main(){
    tcp_allowed_congestion_control();
    return 0;
}

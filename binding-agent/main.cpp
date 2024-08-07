#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

#define MAX_NUM 100

void printSomething00 ();
void printSomething01 ();

int main () {
    std::thread thread_A(printSomething00);
    std::thread thread_B(printSomething01);

    thread_A.join();
    thread_B.join();

    return 0;
}

void printSomething00 () {
    for (int i=0; i < MAX_NUM; i++) {
        printf("print 00 - %d", i);
        sleep(1);
    }
}

void printSomething01 () {
    for (int i=0; i < MAX_NUM; i++) {
        printf("print 01 - %d", i);
        sleep(1);
    }
}
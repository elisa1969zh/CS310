#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void one(void* arg) {                                                            
    thread_lock(0);                                                                     
    printf("1 wait...\n");
    thread_wait(0,0);
    printf("1 awake...\n");
    thread_unlock(0);
    printf("1 done...\n");
}

void two(void* arg) {
    thread_lock(0);
    printf("2 yields...\n");
    thread_yield();
    printf("2 return\n");
    thread_unlock(0);
    printf("2 done\n");
    return;
}

void three(void* arg) {
    thread_signal(0,0);
    printf("3 signals...\n");
    thread_lock(0);
    printf("3 locks\n");
    thread_unlock(0);
    printf("3 done\n");
}

void all_thread_creation (void* arg) {
    thread_create((thread_startfunc_t) one, arg);
    thread_create((thread_startfunc_t) two, arg);
    thread_create((thread_startfunc_t) three, arg);
    printf("created all threads\n");
}

int main(int argc, char *argv[]) {
    int valBack =  thread_libinit((thread_startfunc_t) all_thread_creation, (void*) NULL);
    // printf("ended program\n");
    return 1;
}
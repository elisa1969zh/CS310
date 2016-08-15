#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void one(void* arg) {                                                            
    thread_lock(0);
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    cout << "1 yielding...\n";
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    cout << "1 yielding...\n";
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_yield();
    thread_unlock(0);
    cout << "1 done...\n";
}

void all_thread_creation (void* arg) {
    thread_create((thread_startfunc_t) one, arg);
    cout << "created all threads\n";
}

int main(int argc, char *argv[]) {
    int valBack =  thread_libinit((thread_startfunc_t) all_thread_creation, (void*) NULL);
    exit(0);
    return 1;
}
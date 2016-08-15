#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void one(void* arg) {
    if (thread_lock(1) == -1) {
        cout << "thread_lock failed incorrectly!\n";
    }
    
    if (thread_lock(1) == -1) {
        cout << "thread_lock failed correctly!\n";
    }
    
    else {
        cout << "thread_lock passed incorrectly!\n";
    }
    
    if (thread_unlock(1) == -1) {
        cout << "thread_unlock failed incorrectly!\n";
    }
    
    if (thread_unlock(1) == -1) {
        cout << "thread_unlock failed correctly!\n";
    }
    
    else {
        cout << "thread_unlock passed incorrectly!\n";
    }
    
    if (thread_wait(1,1) == -1) {
        cout << "thread_wait failed correctly!\n";
    }
    
    else {
        cout << "thread_wait failed incorrectly!\n";
    }
}

int main(int argc, char *argv[]) {
    thread_libinit((thread_startfunc_t) one, (void*) NULL);
    exit(0);
    
    return 1;
}
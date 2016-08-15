#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void one(void* arg) {
    if (thread_libinit((thread_startfunc_t) one, (void*) NULL) == -1) {
        cout << "thread_libinit failed correctly!\n";
    }
    
    thread_lock(1);
    cout <<"1 wait...\n";
    cout <<"1 awake...\n";
    cout <<"1 done...\n";
    thread_unlock(1);
    cout <<"Ended program\n";
}

int main(int argc, char *argv[]) {
    if (thread_libinit((thread_startfunc_t) one, (void*) NULL) == -1) {
        cout << "thread_libinit failed incorrectly!\n";
    }
    
    exit(0);
    
    return 1;
}
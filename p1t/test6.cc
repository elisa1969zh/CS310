#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void one(void* arg) {
    thread_lock(1);
    cout <<"1 wait...\n";
    cout <<"1 awake...\n";
    cout <<"1 done...\n";
    thread_unlock(1);
    cout <<"Ended program\n";
}

int main(int argc, char *argv[]) {
    if (thread_create((thread_startfunc_t) one, (void*) NULL) == -1) {
        cout << "thread_create correct!\n";
    }
    
    else {
        cout << "thread_create not correct!\n";
        exit(0);
    }
    
    if (thread_yield() == -1) {
        cout << "thread_yield correct!\n";
    }
    
    else {
        cout << "thread_yield not correct!\n";
        exit(0);
    }
    
    if (thread_lock(0) == -1) {
        cout << "thread_lock correct!\n";
    }
    
    else {
        cout << "thread_lock not correct!\n";
        exit(0);
    }
    
    if (thread_unlock(0) == -1) {
        cout << "thread_unlock correct!\n";
    }
    
    else {
        cout << "thread_unlock not correct!\n";
        exit(0);
    }
    
    if (thread_wait(0,0) == -1) {
        cout << "thread_wait correct!\n";
    }
    
    else {
        cout << "thread_wait not correct!\n";
        exit(0);
    }
    
    if (thread_signal(0,0) == -1) {
        cout << "thread_signal correct!\n";
    }
    
    else {
        cout << "thread_signal not correct!\n";
        exit(0);
    }
    
    if (thread_broadcast(0,0) == -1) {
        cout << "thread_broadcast correct!\n";
    }
    
    else {
        cout << "thread_broadcast not correct!\n";
        exit(0);
    }
    
    exit(0);
    
    return 1;
}
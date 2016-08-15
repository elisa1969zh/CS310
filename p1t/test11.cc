#include <stdio.h>
#include <stdlib.h> 
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <fstream>
#include <limits.h>

using namespace std;

unsigned int mainLock = 1;
unsigned int secondLock = 2;
unsigned int deepV = 3;
void SecondThread(void*);

void TestThread(void* args) {
        thread_lock(mainLock);
        thread_create((thread_startfunc_t) SecondThread, (void*) 0);
        thread_lock(secondLock);
        thread_wait(mainLock, deepV);
        cout << "Mainlock returned\n";
        thread_unlock(secondLock);
        thread_unlock(mainLock);
        cout << "TEST FAILED!" << endl;
}

void SecondThread(void* args) {
        thread_lock(secondLock);
        thread_lock(mainLock);
        thread_signal(mainLock, deepV);
        thread_unlock(mainLock);
        thread_unlock(secondLock);
}



int main(int argc, char *argv[]){
    thread_libinit((thread_startfunc_t) TestThread,((void*) argv));
    return 0;
}

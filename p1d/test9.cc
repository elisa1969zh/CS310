#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void one(void* arg) {

	thread_yield();  
	cout << "waiting on 1\n";                                                          
    thread_wait(0,0);
    cout << "locking in thread 1\n";
    thread_lock(2);
    cout << "yielding in thread1\n";
    thread_yield(); // should lock in thread 1 then proceed to thread 1 again
    thread_broadcast(0,0);
    thread_wait(2,0);
}

void two(void* arg) {
	cout << "waiting on 2\n";                                                            
    thread_wait(0,0);
    cout << "returned from wait\n";
    thread_wait(0,0);
    cout << "broadcasting";
    thread_broadcast(2,0);
}

void three(void* arg) {
	cout << "broadcasting for 1 and 2\n";                                                            
    thread_broadcast(0,0);
    thread_wait(0,0);
    cout << "deadlocking thread 3\n";
    thread_wait(2,0);
}

void all_thread_creation (void* arg) {
    //start_preemptions(true, false, 1);
    thread_create((thread_startfunc_t) one, arg);
    thread_create((thread_startfunc_t) two, arg);
    thread_create((thread_startfunc_t) three, arg);
    thread_create((thread_startfunc_t) one, arg);;
    cout <<"created all threads\n";
    
}

int main(int argc, char *argv[]) {
    int valBack =  thread_libinit((thread_startfunc_t) all_thread_creation, (void*) NULL);
    exit(0);
    return 1;
}
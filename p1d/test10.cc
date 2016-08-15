//test10.cc
#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;
static int globN = 0;

void one(void* arg) {                                                            
    thread_lock(0); 
    //cout <<"can a thread hold two locks?\n";  
    if(thread_lock(1) == 0){
    	cout << "1 holds two lock\n";
    }else{
    	cout << "1 did not acquire second lock\n";
    }                                                                  
    cout <<"1 wait...\n";
    thread_wait(0,0);
    cout <<"1 returned but lock1 is still locked\n";
    thread_unlock(1);
    // should not go back on ready queue - thread is still locked
    if ( thread_signal(4, 0) == 0){
    	cout << "thread 1 was able to signal an unseen signal\n";
    }else{
    	cout << "thread 1 was unable to signal\n";
    }

    cout << "thread 1 dies\n";
}

void two(void* arg) {
    if( thread_unlock(1) == 0){
    	cout << "thread 2's unlock was successful\n";
    }
    else{
    	cout << "thread 2 couldn't unlock\n";
    }
    // can I unlock if I don't hold the lock
    thread_signal(0,0);
    thread_lock(0);
    cout << "thread 2 yields\n";
    thread_yield();
    cout << "thread 2 returns\n";
    thread_unlock(0);
    cout << "thread 2 dies\n";
    if ( thread_wait(3, 0) == 0){
    	cout << "thread 2 on wait queue\n";
    }else{
    	cout << "thread 2 couldn't be put on wait queue\n";
    }

    return;
}

void three(void* arg) {
	cout << "3 attempts lock\n";
    thread_lock(1);
    int n = globN;
    while(n < 5){
    	n++;
    	thread_yield();
    	cout << n-- << "\n" ;
    	globN = n++;
    }
    thread_unlock(1);
    if(thread_lock(-1) == 0){
    	cout << "can lock negative numbers\n";
    	thread_unlock(-1);

    }else{
    	cout << "can't lock negative numbers\n";
    }
}

void four(void* arg){
	int n = globN;
	while(n < 3){
		cout << n++ << "\n" ;
		if(n%2 == 0){
			thread_yield();
		}
	}
	
}

void all_thread_creation (void* arg) {
    thread_create((thread_startfunc_t) one, arg);
    cout << "thread creator yielding\n";
    thread_yield();
    cout << "thread creator returned\n";
    thread_create((thread_startfunc_t) two, arg);
    cout << "thread creator locked\n";
    thread_lock(1);
    thread_create((thread_startfunc_t) three, arg);
    cout << "thread creator yielding again\n";
    thread_yield();
    thread_unlock(1);
    cout << "thread creator yielding again\n";
    thread_yield();
 
    thread_create((thread_startfunc_t) four, arg);
    cout <<"created all threads\n";
}

int main(int argc, char *argv[]) {
    int valBack =  thread_libinit((thread_startfunc_t) all_thread_creation, (void*) NULL);
    exit(0);
    return 1;
}
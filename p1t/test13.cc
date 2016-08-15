#include "thread.h"
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void func(void* arg){
	int i = 1;
}
void one(void* arg){
	for(int i = 0; i< 1000; i++){
		printf("%d ", i);
		if(thread_create(func, arg) == -1){
			printf("thread create failed\n");
		};
		thread_yield();
	}
}

int main(int argc, char *argvp[]){
	thread_libinit((thread_startfunc_t) one, (void*) NULL);
	exit(0);

	return 1;
}

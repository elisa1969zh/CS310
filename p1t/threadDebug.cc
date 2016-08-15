#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "thread.h"
#include "interrupt.h"
#include <deque>
#include <assert.h>
#include <map>

using namespace std;
using std::deque;
using std::pair;
using std::map;

static ucontext_t* CURRENT_RUNNING_THREAD;

static std::deque<ucontext_t*> READY_QUEUE;                                                             // READY QUEUE
static std::map<pair<unsigned int, unsigned int>, deque<ucontext_t*> > MAP_OF_WAIT_QUEUE;               // WAIT QUEUES
static std::map<unsigned int, deque<ucontext_t*> > MAP_OF_LOCK_QUEUE;                                   // LOCK QUEUES
static std::map<unsigned int, ucontext_t*> MAP_OF_LOCK_OWNER;                                           // QUEUE OWNERS

static bool interruptsEnabled = true;

void SWITCH() {
    printf("Trying to switch\n");
    
    printf("Ready Queue Size: %lu\n", READY_QUEUE.size());
    for( int i = 0; i< READY_QUEUE.size(); i++){
        printf("%p\n", READY_QUEUE[i]);
    }

    if (READY_QUEUE.size() > 1) {                                                          // if (got thread)
        ucontext_t* NEXT_THREAD = READY_QUEUE.front();                                     // pick a thread TCB from ready list; 
        ucontext_t* OLD_THREAD  = CURRENT_RUNNING_THREAD;
        
        CURRENT_RUNNING_THREAD = NEXT_THREAD;                                      
        READY_QUEUE.pop_front();     
        if(!interruptsEnabled){
            interrupt_enable();
            interruptsEnabled = true;
            // if we're about to switch, this is where we should
            // finally allow interrupts I believe
        }                                                    // remove next thread from ready queue;
        swapcontext(OLD_THREAD, NEXT_THREAD);                                              // swap context; (save my context;  load saved context for thread; )
    }
}

void context_creation (ucontext_t* thread) {
    //printf("Trying to create context\n");
    getcontext(thread);                                                 
    char *stack = new char [STACK_SIZE];
    thread->uc_stack.ss_sp = stack;                                                            // Allocate a new stack
    thread->uc_stack.ss_size = STACK_SIZE;                                          
    thread->uc_stack.ss_flags = 0;
    thread->uc_link = NULL;
    //printf("Exiting Create context\n");
}

int thread_libinit(thread_startfunc_t func, void *arg) {

    // INITIALIZE THREAD AND ASSIGN VALUES APPROPRIATELY
    printf("Entering LibInit\n");
    ucontext_t *thread = new ucontext_t();
    context_creation(thread);
    makecontext(thread, (void(*)()) func, 1, arg);
    //
    
    CURRENT_RUNNING_THREAD = thread;
    
    func(arg);
    SWITCH();
    cout << "Thread library exiting.";
    //exit(0);
    return 0;
}

int thread_create(thread_startfunc_t func, void *arg) {

    // INITIALIZE THREAD AND ASSIGN VALUES APPROPRIATELY
    printf("Creating Thread\n");
    ucontext_t *thread = new ucontext_t();
    context_creation(thread);
    makecontext(thread, (void(*)()) func, 1, arg);
    //

    READY_QUEUE.push_back(thread);
    printf("Leaving ThreadCreate\n");

    return 0;
}

int thread_yield(void) {
    printf("Attempting yield\n");
    interrupt_disable();
    interruptsEnabled = false;

    READY_QUEUE.push_back(CURRENT_RUNNING_THREAD);                                              // put my TCB on ready list
    SWITCH();    
    interrupt_disable();
    interruptsEnabled = false;                                                                               // switch(); 

    interrupt_enable();
    interruptsEnabled = true;
    printf("Ending yield\n");
    return 0;
}

int thread_lock(unsigned int lock) {
    printf("Attempting lock\n");
    if (MAP_OF_LOCK_QUEUE.count(lock) == 0) {                                                    // it's a new lock
        std::deque<ucontext_t*> LOCK_QUEUE;
        MAP_OF_LOCK_QUEUE[lock] = LOCK_QUEUE;
        MAP_OF_LOCK_OWNER[lock] = NULL;
    }
    
    while (MAP_OF_LOCK_OWNER[lock] != NULL) {                                                   // while (this monitor is not free) { 
        MAP_OF_LOCK_QUEUE[lock].push_back(CURRENT_RUNNING_THREAD);
        printf("couldn't give LOCK");                             // wput my TCB on this monitor lock list; 
        SWITCH();                                                                               /* sleep */ 
    }
    
    MAP_OF_LOCK_OWNER[lock] = CURRENT_RUNNING_THREAD;
    printf("Gave lock\n");     
                   // set this thread as owner of monitor; 
    return 0;
}

int thread_unlock(unsigned int lock) {
    printf("Entering Unlock\n");   
    MAP_OF_LOCK_OWNER[lock] = NULL;                                                         // set this monitor free;
    
    if (MAP_OF_LOCK_QUEUE[lock].size() > 0) {                                               // if there are threads on the lock queue
        READY_QUEUE.push_back(MAP_OF_LOCK_QUEUE[lock].front());                             // put a waiter TCB on ready list;
        MAP_OF_LOCK_QUEUE[lock].pop_front();                                                // get waiter TCB off this monitor lock list; 
    }
    printf("Leaving Unlock\n");   
    return 0;
}

int thread_wait(unsigned int lock, unsigned int cond) {
    printf("Entering Wait\n");   
    interrupt_disable();
    interruptsEnabled = false;

    thread_unlock(lock);                                                                    // unlock(); 
    
    std::pair<unsigned int, unsigned int> LOCK_COND_PAIR (lock, cond);
    
    if (MAP_OF_WAIT_QUEUE.count(LOCK_COND_PAIR) == 0) {                                     // if this is a new conditional variable
        std::deque<ucontext_t*> WAIT_QUEUE;
        MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR] = WAIT_QUEUE;
    }
    
    MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].push_back(CURRENT_RUNNING_THREAD);                    // put my TCB on this monitor wait list;
    SWITCH();  
    interrupt_disable();
    interruptsEnabled = false;

    thread_lock(lock);                                                                      // lock(); 

    interrupt_enable();
    interruptsEnabled = true;
    printf("Leaving Wait\n");   
    return 0;
}

int thread_signal(unsigned int lock, unsigned int cond) {
        printf("Entering Signal\n");   
    std::pair<unsigned int, unsigned int> LOCK_COND_PAIR (lock, cond);                      
    
    if (MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].size() > 0) {
        READY_QUEUE.push_back(MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].front());                   // put a waiter TCB on ready list; 
        MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].pop_front();                                      // get waiter TCB off this monitor wait list
    }
        printf("Leaving Signal\n");   
    return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond) {
        printf("Entering Broadcast\n");   
    std::pair<unsigned int, unsigned int> LOCK_COND_PAIR (lock, cond);
    
    if (MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].size() > 0) {                                     // repeat thread_signal for however many threads on wait queue
        for (int i = 0; i < MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].size(); i++) {    
            thread_signal(lock, cond);
        }
    }
        printf("Leaving Broadcast");
    return 0;
}


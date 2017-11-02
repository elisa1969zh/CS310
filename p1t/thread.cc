#define _XOPEN_SOURCE 600
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc/malloc.h>
#include "thread.h"
#include "interrupt.h"
#include <deque>
#include <map>
#include <iostream>
#include <assert.h>

using namespace std;
using std::deque;
using std::pair;
using std::map;

static ucontext_t *CURRENT_RUNNING_THREAD;
static ucontext_t *EXIT_HANDLER_THREAD;
static ucontext_t *THREAD_TO_DELETE;
static ucontext_t *THREAD_TO_SWAP_AFTER_DELETE;
static ucontext_t *LIBINIT_THREAD;

static int thread_libinit_called = 0;

static std::deque<ucontext_t *> READY_QUEUE; // READY QUEUE
static std::map <pair<unsigned int, unsigned int>, deque<ucontext_t *>>
    MAP_OF_WAIT_QUEUE; // WAIT QUEUES
static std::map<unsigned int, deque<ucontext_t *> >
    MAP_OF_LOCK_QUEUE; // LOCK QUEUES
static std::map<unsigned int, ucontext_t *> MAP_OF_LOCK_OWNER; // QUEUE OWNERS
static std::map<unsigned int, unsigned int> COND_LOCK_UNIQUE;

static std::map<ucontext_t *, bool> IN_WAIT_OR_NOT;
static std::map<ucontext_t *, bool> IN_BROADCAST_OR_NOT;

static std::map<ucontext_t *, bool> IS_BLOCKED;

static bool interruptsEnabled = true;
static int TOTAL_NUM_OF_THREADS = 0;

static void INTERRUPT_ENABLE() { // simple helper method
  while (!interruptsEnabled) {
    interruptsEnabled = true; // ORDER MATTERS
    interrupt_enable();
  }

  // assert_interrupts_enabled();
}

static void INTERRUPT_DISABLE() { // simple helper method
  while (interruptsEnabled) {
    interrupt_disable();
    interruptsEnabled = false; // ORDER MATTERS
  }

  // assert_interrupts_disabled();
}

static void print_all_three_queues_and_info(void) { // PRINT READY QUEUE
  printf("Ready Queue Size: %lu: [", READY_QUEUE.size());

  for (int i = 0; i < READY_QUEUE.size(); i++) {
    printf("%p ", READY_QUEUE[i]);
  }

  printf("]\n");

  // PRINT LOCK QUEUE
  printf("Lock Queue Size: %lu: [", MAP_OF_LOCK_QUEUE.size());

  for (std::map < unsigned int, deque < ucontext_t * >> ::iterator i =
                                                                       MAP_OF_LOCK_QUEUE.begin();
  i != MAP_OF_LOCK_QUEUE.end();
  i++) {
    std::cout << i->first << " => [";

    for (int j = 0; j < i->second.size(); j++) {
      printf("%p ", i->second[j]);
    }

    printf("]\n");
  }

  printf("]\n");

  // PRINT WAIT QUEUE
  printf("Wait Queue Size: %lu: [", MAP_OF_WAIT_QUEUE.size());

  for (std::map <pair<unsigned int, unsigned int>, deque<ucontext_t *>>
      ::iterator
  i = MAP_OF_WAIT_QUEUE.begin();
  i != MAP_OF_WAIT_QUEUE.end();
  i++) {
    std::cout << "(" << i->first.first << ", " << i->first.second << ")"
              << " => [";

    for (int j = 0; j < i->second.size(); j++) { printf("%p  ", i->second[j]); }

    printf("]\n");
  }

  printf("]\n");

  // PRINT LOCK_OWNERS
  printf("LOCK AND LOCK OWNERS: %lu: [", MAP_OF_LOCK_OWNER.size());

  for (std::map < unsigned int, ucontext_t * > ::iterator i =
                                                              MAP_OF_LOCK_OWNER.begin();
  i != MAP_OF_LOCK_OWNER.end();
  i++) {
    std::cout << "(Lock" << i->first << " : owner -> " << i->second << ") ";
  }

  printf("]\n");

  printf("CURRENT_RUNNING_THREAD: %p\n", CURRENT_RUNNING_THREAD);

  printf("INTERRUPT IS = %s\n\n", interruptsEnabled ? "ENABLED" : "DISABLED");

}

static int EXIT_HANDLER_THREAD_FUNC(void) {
  while (true) {
    if (THREAD_TO_DELETE != NULL) {
      if (THREAD_TO_DELETE->uc_stack.ss_sp
          != NULL) { delete (char *) THREAD_TO_DELETE->uc_stack.ss_sp; }

      delete THREAD_TO_DELETE;

    }

    TOTAL_NUM_OF_THREADS += -1;
    // printf("TOTAL_NUM_OF_THREADS: %d\n", TOTAL_NUM_OF_THREADS);
    // printf("THREAD DELETED: %p\n\n", THREAD_TO_DELETE);

    if (swapcontext(EXIT_HANDLER_THREAD, THREAD_TO_SWAP_AFTER_DELETE)
        == -1) { return -1; }
  }

  return 0;
}

static int SWITCH(bool THREAD_EXIT_OR_NOT) {
  INTERRUPT_DISABLE();

  //print_all_three_queues_and_info();

  if (!READY_QUEUE.empty()) { // if (got thread)

    ucontext_t *NEXT_THREAD =
        READY_QUEUE.front(); // pick a thread TCB from ready list;
    ucontext_t *OLD_THREAD = CURRENT_RUNNING_THREAD;

    CURRENT_RUNNING_THREAD = NEXT_THREAD;
    READY_QUEUE.pop_front(); // remove next thread from ready queue;

    if (THREAD_EXIT_OR_NOT == true) {
      THREAD_TO_DELETE = OLD_THREAD;
      THREAD_TO_SWAP_AFTER_DELETE = NEXT_THREAD;

      if (swapcontext(OLD_THREAD, EXIT_HANDLER_THREAD) == -1) { return -1; }
    } else {
      if (swapcontext(OLD_THREAD, NEXT_THREAD)
          == -1) { return -1; }                // swap context; (save my context;  load saved context for thread; )
    }
  } else {
    if (THREAD_EXIT_OR_NOT == true) {
      //exiting a thread when the ready queue is empty
      // exit program
      INTERRUPT_ENABLE(); /*need to discuss about this*/
      cout << "Thread library exiting.\n";
      exit(0);
    } else {
      if (IS_BLOCKED[CURRENT_RUNNING_THREAD]) {
        // if current_running_thread is busy
        INTERRUPT_ENABLE(); /*need to discuss about this*/
        cout << "Thread library exiting.\n";
        exit(0);
      }

      THREAD_TO_DELETE = CURRENT_RUNNING_THREAD;
    }

    //if (swapcontext(CURRENT_RUNNING_THREAD, LIBINIT_THREAD) == -1) {return -1;}
    //}
  }

  return 0;
}

static int context_creation(ucontext_t *thread) {
  if (getcontext(thread) == -1) { return -1; }

  try {
    char *stack = new char[STACK_SIZE];
    thread->uc_stack.ss_sp = stack; // Allocate a new stack
    thread->uc_stack.ss_size = STACK_SIZE;
    thread->uc_stack.ss_flags = 0;
    thread->uc_link = NULL;

    if (stack == NULL) { return -1; }
  }

  catch (std::bad_alloc &ba) { return -1; }

  TOTAL_NUM_OF_THREADS += 1;
  // printf("TOTAL_NUM_OF_THREADS: %d\n", TOTAL_NUM_OF_THREADS);
  // printf("THREAD CREATED: %p\n\n", thread);
  IS_BLOCKED[thread] = false;
  IN_WAIT_OR_NOT[thread] = false;
  IN_BROADCAST_OR_NOT[thread] = false;

  return 0;
}

int thread_libinit(thread_startfunc_t func, void *arg) {
  INTERRUPT_DISABLE();
  thread_libinit_called += 1;

  if (thread_libinit_called > 1) {
    INTERRUPT_ENABLE();
    return -1;
  }

  // LIBINIT_THREAD //
  try { LIBINIT_THREAD = new ucontext_t(); }

  catch (std::bad_alloc &ba) {
    INTERRUPT_ENABLE();
    return -1;
  }

  if (LIBINIT_THREAD == NULL) {
    INTERRUPT_ENABLE();
    return -1;
  }

  if (context_creation(LIBINIT_THREAD) == -1) {
    INTERRUPT_ENABLE();
    return -1;
  }

  makecontext(LIBINIT_THREAD, (void (*)()) func, 1, arg);

  // EXIT_HANDLER_THREAD //

  try { EXIT_HANDLER_THREAD = new ucontext_t(); }

  catch (std::bad_alloc &ba) {
    INTERRUPT_ENABLE();
    return -1;
  }

  if (context_creation(EXIT_HANDLER_THREAD) == -1) {
    INTERRUPT_ENABLE();
    return -1;
  }

  makecontext(EXIT_HANDLER_THREAD, (void (*)()) EXIT_HANDLER_THREAD_FUNC, 0);

  if (EXIT_HANDLER_THREAD == NULL) {
    INTERRUPT_ENABLE();
    return -1;
  }

  // printf("EXIT_HANDLER_THREAD created\n\n");

  CURRENT_RUNNING_THREAD = LIBINIT_THREAD;

  INTERRUPT_ENABLE();
  func(arg);
  INTERRUPT_DISABLE();

  if (SWITCH(true) == -1) { return -1; }

}

static int STUB(thread_startfunc_t func, void *arg) {
  INTERRUPT_ENABLE();
  func(arg);
  INTERRUPT_DISABLE();

  if (SWITCH(true) == -1) { return -1; }
}

int thread_create(thread_startfunc_t func, void *arg) {
  INTERRUPT_DISABLE();

  if (thread_libinit_called == 0) {
    INTERRUPT_ENABLE();
    return -1;
  }

  try {
    ucontext_t *thread = new ucontext_t();

    if (context_creation(thread) == -1) {
      INTERRUPT_ENABLE();
      return -1;
    }

    makecontext(thread, (void (*)()) STUB, 2, func, arg);

    READY_QUEUE.push_back(thread); // Add thread to ready queue

    if (thread == NULL) {
      INTERRUPT_ENABLE();
      return -1;
    }
  }

  catch (std::bad_alloc &ba) {
    INTERRUPT_ENABLE();
    return -1;
  }

  INTERRUPT_ENABLE();
  return 0;
}

int thread_yield(void) {
// printf("----/*/**/*/------------thread_yield------------\n");
  INTERRUPT_DISABLE();

  if (thread_libinit_called == 0) {
    INTERRUPT_ENABLE();
    return -1;
  }

  READY_QUEUE.push_back(CURRENT_RUNNING_THREAD); // put my TCB on ready list

  if (SWITCH(false) == -1) {
    INTERRUPT_ENABLE();
    return -1;
  }

  INTERRUPT_ENABLE();
  return 0;
}

int thread_lock(unsigned int lock) {
  INTERRUPT_DISABLE();

  if ((thread_libinit_called == 0)
      || (MAP_OF_LOCK_OWNER[lock] == CURRENT_RUNNING_THREAD)) {
    INTERRUPT_ENABLE();
    return -1;
  }

  while (MAP_OF_LOCK_QUEUE.count(lock) == 0) { // it's a new lock
    std::deque < ucontext_t * > LOCK_QUEUE;
    MAP_OF_LOCK_QUEUE[lock] = LOCK_QUEUE;
    MAP_OF_LOCK_OWNER[lock] = NULL;
  }

  while ((MAP_OF_LOCK_OWNER[lock] != NULL) && (MAP_OF_LOCK_OWNER[lock]
      != CURRENT_RUNNING_THREAD)) { // while (this monitor is not free) {
    // put my TCB on this monitor lock list;
    MAP_OF_LOCK_QUEUE[lock].push_back(CURRENT_RUNNING_THREAD);
    IS_BLOCKED[CURRENT_RUNNING_THREAD] = true;

    if (SWITCH(false) == -1) {
      INTERRUPT_ENABLE();
      return -1;
    }
  }

  if (MAP_OF_LOCK_OWNER[lock] == NULL) {
    MAP_OF_LOCK_OWNER[lock] = CURRENT_RUNNING_THREAD;
    // unblock the thread
  }        // set this thread as owner of monitor;

  if (!IN_WAIT_OR_NOT[CURRENT_RUNNING_THREAD]) { INTERRUPT_ENABLE(); }

  return 0;
}

int thread_unlock(unsigned int lock) {
  INTERRUPT_DISABLE();

  if ((thread_libinit_called == 0)
      || (MAP_OF_LOCK_OWNER[lock] != CURRENT_RUNNING_THREAD)
      || MAP_OF_LOCK_OWNER.count(lock) == 0) {
    INTERRUPT_ENABLE();
    return -1;
  }

  MAP_OF_LOCK_OWNER[lock] = NULL; // set this monitor free;

  if (!MAP_OF_LOCK_QUEUE[lock].empty()) { // if there are threads on the lock queue
    READY_QUEUE.push_back(MAP_OF_LOCK_QUEUE[lock].front()); // put a waiter TCB on ready list;
    IS_BLOCKED[MAP_OF_LOCK_QUEUE[lock].front()] = false; // unblock the thread

    MAP_OF_LOCK_OWNER[lock] = MAP_OF_LOCK_QUEUE[lock].front();
    MAP_OF_LOCK_QUEUE[lock].pop_front(); // get waiter TCB off this monitor lock list;
  }

  if (!IN_WAIT_OR_NOT[CURRENT_RUNNING_THREAD]) { INTERRUPT_ENABLE(); }

  return 0;
}

int thread_wait(unsigned int lock, unsigned int cond) {
  INTERRUPT_DISABLE();

  if (thread_libinit_called == 0) {
    INTERRUPT_ENABLE();
    return -1;
  }

  //if ((COND_LOCK_UNIQUE.count(cond) != 0) && (COND_LOCK_UNIQUE[cond] != lock)) {INTERRUPT_ENABLE(); return -1;}

  //if (COND_LOCK_UNIQUE.count(cond) == 0) {COND_LOCK_UNIQUE[cond] = lock;}

  IN_WAIT_OR_NOT[CURRENT_RUNNING_THREAD] = true;

  if (thread_unlock(lock) == -1) {
    INTERRUPT_ENABLE();
    return -1;
  }                                             // unlock();

  std::pair<unsigned int, unsigned int> LOCK_COND_PAIR(lock, cond);

  if (MAP_OF_WAIT_QUEUE.count(LOCK_COND_PAIR)
      == 0) {                                     // if this is a new conditional variable
    std::deque < ucontext_t * > WAIT_QUEUE;
    MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR] = WAIT_QUEUE;
  }

  MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].push_back(CURRENT_RUNNING_THREAD);
  IS_BLOCKED[CURRENT_RUNNING_THREAD] = true;  // unblock the thread
  // put my TCB on this monitor wait list;

  if (SWITCH(false) == -1) {
    INTERRUPT_ENABLE();
    return -1;
  }

  if (thread_lock(lock) == -1) {
    INTERRUPT_ENABLE();
    return -1;
  }  // lock();

  IN_WAIT_OR_NOT[CURRENT_RUNNING_THREAD] = false;

  INTERRUPT_ENABLE();
  return 0;
}

int thread_signal(unsigned int lock, unsigned int cond) {
  INTERRUPT_DISABLE();

  if (thread_libinit_called == 0) {
    INTERRUPT_ENABLE();
    return -1;
  }

  //if ((COND_LOCK_UNIQUE.count(cond) != 0) && (COND_LOCK_UNIQUE[cond] != lock)) {INTERRUPT_ENABLE(); return -1;}

  //if (COND_LOCK_UNIQUE.count(cond) == 0) {COND_LOCK_UNIQUE[cond] = lock;}

  std::pair<unsigned int, unsigned int> LOCK_COND_PAIR(lock, cond);

  if (!MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].empty()) {
    READY_QUEUE.push_back(MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].front()); // put a waiter TCB on ready list;
    IS_BLOCKED[MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].front()] =
        false; // unblock the thread
    MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].pop_front(); // get waiter TCB off this monitor wait list
  }

  if (!IN_BROADCAST_OR_NOT[CURRENT_RUNNING_THREAD]) { INTERRUPT_ENABLE(); }

  return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond) {
  INTERRUPT_DISABLE();

  if (thread_libinit_called == 0) {
    INTERRUPT_ENABLE();
    return -1;
  }

  //if ((COND_LOCK_UNIQUE.count(cond) != 0) && (COND_LOCK_UNIQUE[cond] != lock)) {INTERRUPT_ENABLE(); return -1;}

  //if (COND_LOCK_UNIQUE.count(cond) == 0) {COND_LOCK_UNIQUE[cond] = lock;}

  IN_BROADCAST_OR_NOT[CURRENT_RUNNING_THREAD] = true;

  std::pair<unsigned int, unsigned int> LOCK_COND_PAIR(lock, cond);

  while (!MAP_OF_WAIT_QUEUE[LOCK_COND_PAIR].empty()) {
    // repeat thread_signal for however many threads on wait queue
    if (thread_signal(lock, cond) == -1) {
      INTERRUPT_ENABLE();
      return -1;
    }
  }

  IN_BROADCAST_OR_NOT[CURRENT_RUNNING_THREAD] = false;
  INTERRUPT_ENABLE();
  return 0;
}


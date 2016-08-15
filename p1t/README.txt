-------------------user error detection-----------------------

getcontext error
makecontext error

calling another thread function before thread_libinit: 									thread_init_called = 0;
calling thread_libinit more than once: 													thread_init_called > 1;
misusing monitors: wait (lock), wait (unlock)
thread tries to acquire a lock it already has or release a lock it doesn't have:		MAP_OF_LOCK_OWNER[lock] == CURRENT_RUNNING_THREAD
																						MAP_OF_LOCK_OWNER[lock] != CURRENT_RUNNING_THREAD

---------test case suites--------------

(completed) test case 1: alex and jiawei's test case																			
(completed) test case 2: deli test case																			
(completed) test case 3: app.cc 																			
(completed) test case 4: simple single thread program with many thread_yield																			
(completed) test case 5: deli test case with thread_broadcast wiht many thread_yield																			
(completed) test case 6: thread library detects program calling another thread function before thread_libinit
(completed) test case 7: thread library detects program calling thread_libinit more than once 

(completed) test case 8: thread library detects thread tries to acquire a lock it already has or release a lock it doesn't have
(completed) test case 9: testing broadcast fidelity
(completed) test case 10: modified test case 1 to detect whether thread libraries can handle various errors

(to be completed) test case 11: thread library detects misusing monitors 


----------------autograder test case purposes----------------
------------------------- 0 ------------------------- 
------------------------- 1 ------------------------- 
------------------------- 2 ------------------------- 
------------------------- 3 ------------------------- asynchronous preemption
------------------------- 4 ------------------------- asynchronous preemption
------------------------- 5 ------------------------- 
------------------------- 6 ------------------------- 
------------------------- 7 ------------------------- 
------------------------- 8 ------------------------- 
------------------------- 9 ------------------------- 
------------------------- 10 ------------------------- 
------------------------- 11 ------------------------- 
------------------------- 12 ------------------------- 
------------------------- 13 ------------------------- 
------------------------- 14 ------------------------- 
------------------------- 15 ------------------------- 
------------------------- 16 ------------------------- how well your thread library handles errors
------------------------- 17 ------------------------- how well your thread library handles errors
------------------------- 18 ------------------------- 
------------------------- 19 ------------------------- 
------------------------- 20 ------------------------- asynchronous preemption
------------------------- 21 ------------------------- asynchronous preemption
------------------------- 22 ------------------------- asynchronous preemption
------------------------- 23 ------------------------- asynchronous preemption
------------------------- 24 ------------------------- asynchronous preemption
------------------------- 25 ------------------------- 
------------------------- 26 ------------------------- 

-------------------02/21 result -----------------------
------------------------- 0 ------------------------- perfect!
------------------------- 1 ------------------------- perfect!
------------------------- 2 ------------------------- process exited with error
------------------------- 3 ------------------------- perfect!
------------------------- 4 ------------------------- perfect!
------------------------- 5 ------------------------- test case compiled with student thread library generated incorrect output (output line 5)
------------------------- 6 ------------------------- process exited with error
------------------------- 7 ------------------------- perfect!
------------------------- 8 ------------------------- perfect!
------------------------- 9 ------------------------- perfect!
------------------------- 10 ------------------------- test case compiled with student thread library generated incorrect output (output line 11)
------------------------- 11 ------------------------- test case compiled with student thread library generated incorrect output (output line 16)
------------------------- 12 ------------------------- process exited with error
------------------------- 13 ------------------------- process used too much memory
------------------------- 14 ------------------------- process exited with error
------------------------- 15 ------------------------- process used too much memory
------------------------- 16 ------------------------- process exited with error
------------------------- 17 ------------------------- process exited with error
------------------------- 18 ------------------------- process used too much memory
------------------------- 19 ------------------------- process used too much memory
------------------------- 20 ------------------------- process exited with error
------------------------- 21 ------------------------- process exited with error
------------------------- 22 ------------------------- process took too much CPU time
------------------------- 23 ------------------------- process took too much CPU time
------------------------- 24 ------------------------- process used too much memory
------------------------- 25 ------------------------- process took too much CPU time
------------------------- 26 ------------------------- process took too much CPU time

--------------------02/22 result --------------------
------------------------- 0 ------------------------- process exited with error
------------------------- 1 ------------------------- process exited with error
------------------------- 2 ------------------------- process exited with error
------------------------- 3 ------------------------- process exited with error
------------------------- 4 ------------------------- process exited with error
------------------------- 5 ------------------------- process exited with error
------------------------- 6 ------------------------- process exited with error
------------------------- 7 ------------------------- perfect!
------------------------- 8 ------------------------- process exited with error
------------------------- 9 ------------------------- process exited with error
------------------------- 10 ------------------------- process exited with error
------------------------- 11 ------------------------- process exited with error
------------------------- 12 ------------------------- process exited with error
------------------------- 13 ------------------------- process exited with error
------------------------- 14 ------------------------- process exited with error
------------------------- 15 ------------------------- perfect!
------------------------- 16 ------------------------- process exited with error
------------------------- 17 ------------------------- process exited with error
------------------------- 18 ------------------------- process used too much memory
------------------------- 19 ------------------------- process used too much memory
------------------------- 20 ------------------------- process used too much memory
------------------------- 21 ------------------------- process used too much memory
------------------------- 22 ------------------------- perfect!
------------------------- 23 ------------------------- process exited with error
------------------------- 24 ------------------------- process exited with error
------------------------- 25 ------------------------- perfect!
------------------------- 26 ------------------------- perfect!

--------------------02/23 result ---------------------
------------------------- 0 ------------------------- process exited with error
------------------------- 1 ------------------------- process exited with error
------------------------- 2 ------------------------- process exited with error
------------------------- 3 ------------------------- process exited with error
------------------------- 4 ------------------------- process exited with error
------------------------- 5 ------------------------- process exited with error
------------------------- 6 ------------------------- process exited with error
------------------------- 7 ------------------------- perfect!
------------------------- 8 ------------------------- process exited with error
------------------------- 9 ------------------------- process exited with error
------------------------- 10 ------------------------- process exited with error
------------------------- 11 ------------------------- process exited with error
------------------------- 12 ------------------------- process exited with error
------------------------- 13 ------------------------- process exited with error
------------------------- 14 ------------------------- process exited with error
------------------------- 15 ------------------------- perfect!
------------------------- 16 ------------------------- process exited with error
------------------------- 17 ------------------------- process exited with error
------------------------- 18 ------------------------- process used too much memory
------------------------- 19 ------------------------- process used too much memory
------------------------- 20 ------------------------- process used too much memory
------------------------- 21 ------------------------- process used too much memory
------------------------- 22 ------------------------- process exited with error
------------------------- 23 ------------------------- perfect!
------------------------- 24 ------------------------- perfect!
------------------------- 25 ------------------------- process exited with error
------------------------- 26 ------------------------- perfect!


---------------------02/24 result--------------------------------------------------------
------------------------- 0 ------------------------- perfect!
------------------------- 1 ------------------------- perfect!
------------------------- 2 ------------------------- process exited with error
------------------------- 3 ------------------------- perfect!
------------------------- 4 ------------------------- perfect!
------------------------- 5 ------------------------- test case compiled with student thread library generated incorrect output (output line 5)
------------------------- 6 ------------------------- process exited with error
------------------------- 7 ------------------------- perfect!
------------------------- 8 ------------------------- perfect!
------------------------- 9 ------------------------- perfect!
------------------------- 10 ------------------------- perfect!
------------------------- 11 ------------------------- perfect!
------------------------- 12 ------------------------- perfect!
------------------------- 13 ------------------------- test case compiled with student thread library generated incorrect output (output line 8)
------------------------- 14 ------------------------- process exited with error
------------------------- 15 ------------------------- perfect!
------------------------- 16 ------------------------- process exited with error
------------------------- 17 ------------------------- process exited with error
------------------------- 18 ------------------------- perfect!
------------------------- 19 ------------------------- perfect!
------------------------- 20 ------------------------- perfect!
------------------------- 21 ------------------------- perfect!
------------------------- 22 ------------------------- perfect!
------------------------- 23 ------------------------- perfect!
------------------------- 24 ------------------------- perfect!
------------------------- 25 ------------------------- perfect!
------------------------- 26 ------------------------- perfect!


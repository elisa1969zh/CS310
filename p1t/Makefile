all: c 1 2 3 4 5 6 7 8 9 10 deli

c: 
	find -exec touch \{\} \; && cd ../p1d && find -exec touch \{\} \;

1: 1t 1d 
2: 2t 2d 
3: 3t 3d 
4: 4t 4d 
5: 5t 5d 
6: 6t 6d 
7: 7t 7d 
8: 8t 8d 
9: 9t 9d 
10: 10t 10d 

deli: deli_t deli_d

deli_t: deli.cc thread.cc libinterrupt.a
	g++ -o deli thread.cc deli.cc libinterrupt.a -ldl && ./deli 3 sw.in0 sw.in1 sw.in2 sw.in3 sw.in4 

deli_d: deli.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o deli thread.o deli.cc libinterrupt.a -ldl && ./deli 3 sw.in0 sw.in1 sw.in2 sw.in3 sw.in4

1t: test1.cc thread.cc libinterrupt.a
	g++ -o test1 thread.cc test1.cc libinterrupt.a -ldl && ./test1  	

1d: test1.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test1 thread.o test1.cc libinterrupt.a -ldl && ./test1

2t: test2.cc thread.cc libinterrupt.a
	g++ -o test2 thread.cc test2.cc libinterrupt.a -ldl && ./test2  	

2d: test2.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test2 thread.o test2.cc libinterrupt.a -ldl && ./test2

3t: test3.cc thread.cc libinterrupt.a
	g++ -o test3 thread.cc test3.cc libinterrupt.a -ldl && ./test3  	

3d: test3.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test3 thread.o test3.cc libinterrupt.a -ldl && ./test3

4t: test4.cc thread.cc libinterrupt.a
	g++ -o test4 thread.cc test4.cc libinterrupt.a -ldl && ./test4  	

4d: test4.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test4 thread.o test4.cc libinterrupt.a -ldl && ./test4

5t: test5.cc thread.cc libinterrupt.a
	g++ -o test5 thread.cc test5.cc libinterrupt.a -ldl && ./test5  	

5d: test5.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test5 thread.o test5.cc libinterrupt.a -ldl && ./test5

6t: test6.cc thread.cc libinterrupt.a
	g++ -o test6 thread.cc test6.cc libinterrupt.a -ldl && ./test6  	

6d: test6.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test6 thread.o test6.cc libinterrupt.a -ldl && ./test6

7t: test7.cc thread.cc libinterrupt.a
	g++ -o test7 thread.cc test7.cc libinterrupt.a -ldl && ./test7  	

7d: test7.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test7 thread.o test7.cc libinterrupt.a -ldl && ./test7

8t: test8.cc thread.cc libinterrupt.a
	g++ -o test8 thread.cc test8.cc libinterrupt.a -ldl && ./test8  	

8d: test8.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test8 thread.o test8.cc libinterrupt.a -ldl && ./test8

9t: test9.cc thread.cc libinterrupt.a
	g++ -o test9 thread.cc test9.cc libinterrupt.a -ldl && ./test9  	

9d: test9.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test9 thread.o test9.cc libinterrupt.a -ldl && ./test9

10t: test10.cc thread.cc libinterrupt.a
	g++ -o test10 thread.cc test10.cc libinterrupt.a -ldl && ./test10  	

10d: test10.cc thread.o libinterrupt.a
	cd ../p1d && g++ -o test10 thread.o test10.cc libinterrupt.a -ldl && ./test10

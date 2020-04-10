# Multithreading-Quicksort
* The file sort.c contains a procedure which uses the classic quicksort algorithm to sort an array of strings.
* The nature of quicksort is that following its partition step, two recursive calls can be made to sort each partition.
* These two "sub" sorts are independent of each other and can each be given their own thread of execution.
* The threads of course must be synchronized at least to the extent that the user's calling thread does not return until all the "sub" sorts are completed (and any created threads are joined/destroyed).
* I have modifed the sort.c program to make use of as many threads as is specified on the command line, in order to sort a structure of strings taken from the poem.txt file.
* The threads are created and used through using pthread_create() and pthread_join().
* I have built a test bed around the sort program, to test that the code sorts correctly and to time it.
* Additionally I have coded up the library's qsort() implementation as a benchmark for comparison.
* Since the overhead of creating a thread is significant, the multithreaded quicksort only performs better for a certain number of added threads, with it progressivly losing efficiency when a larger then optimal amount of threads are used.
* A define DEBUG flag is included in sort.c to see extra thread creation/joining information, 0 = off, 1 = on.
* Commented out sections in the main will show the total number of words being sorted from poem.txt, and well as the word structures before and after the threaded quicksort and implemented qsort.
## Screenshot
![Alt text](/screenshot/sc1.png?raw=true "sc1")
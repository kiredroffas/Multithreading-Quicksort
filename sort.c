/*  Erik Safford
 *  Multithreading quickSort() w/ Specified Number of Threads
 *  Spring 2019                                               */

#include "sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define SORT_THRESHOLD      40
#define DEBUG 0  //1 for debug output

//---------------------------------QUICKSORT FUNCTIONS----------------------------

typedef struct _sortParams {
    char** array;
    int left;
    int right;
} SortParams;

static int maximumThreads = 1;                             /* maximum # of threads to be used */
int currentThreads = 0;                                    //Keep track of how many threads are currently in use
pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;   //Initialize mutex     

/* This is an implementation of insert sort, which although it is */
/* n-squared, is faster at sorting short lists than quick sort,   */
/* due to its lack of recursive procedure call overhead.          */

static void insertSort(char** array, int left, int right) {
    int i, j;
    for (i = left + 1; i <= right; i++) {
        char* pivot = array[i];
        j = i - 1;
        while (j >= left && (strcmp(array[j],pivot) > 0)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = pivot;
    }
}

/* Recursive quick sort, but with a provision to use */
/* insert sort when the range gets small.            */

static void quickSort(void* p) {
    SortParams* params = (SortParams*) p;
    char** array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;
	
    if (j - i > SORT_THRESHOLD) {           /* if the sort range is substantial, use quick sort */

        int m = (i + j) >> 1;               /* pick pivot as median of         */
        char* temp, *pivot;                 /* first, last and middle elements */
        if (strcmp(array[i],array[m]) > 0) {
            temp = array[i]; array[i] = array[m]; array[m] = temp;
        }
        if (strcmp(array[m],array[j]) > 0) {
            temp = array[m]; array[m] = array[j]; array[j] = temp;
            if (strcmp(array[i],array[m]) > 0) {
                temp = array[i]; array[i] = array[m]; array[m] = temp;
            }
        }
        pivot = array[m];

        for (;;) {
            while (strcmp(array[i],pivot) < 0) i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j],pivot) > 0) j--; /* move j up to first element less than or equal to pivot      */
            if (i < j) {
                char* temp = array[i];      /* if i and j have not passed each other */
                array[i++] = array[j];      /* swap their respective elements and    */
                array[j--] = temp;          /* advance both i and j                  */
            } else if (i == j) {
                i++; j--;
            } else break;                   /* if i > j, this partitioning is done  */
        }
	
	pthread_t leftThread, rightThread;   //Can potentially create 2 threads when quickSort() called
	int lTCheck = 0, rTCheck = 0;        //Flags to check if threads were created so can wait to join
	int error;                           //Variable to check if pthreads work correctly

        SortParams first;        //Set up the first half of the SortParams structure for next quickSort() call
	first.array = array; 
	first.left = left; 
	first.right = j;

	if(currentThreads < maximumThreads) {  //If havnt made maximumThreads or maximumThreads arn't being used
		if((error = pthread_mutex_lock(&threadMutex)) != 0) {    //Lock mutex, so currentThread count is synchronized
			fprintf(stderr,"Error locking mutex, error = %d\n", error);
		}
		if(DEBUG) { printf("Creating thread for left side\n"); } 
		if((error = pthread_create(&leftThread, NULL, (void *)quickSort, &first)) != 0) {	//Create thread for left half
			fprintf(stderr,"Error creating leftThread, error = %d\n", error);
		}
		currentThreads++;                    //Increment total threads
		lTCheck = 1;                         //Set flag to wait to join
		if((error = pthread_mutex_unlock(&threadMutex)) != 0) {  //Unlock mutex
			fprintf(stderr,"Error unlocking mutex, error = %d\n", error);
		}
	}
	else {  //Otherwise quickSort() with the same thread
        	quickSort(&first);                  /* sort the left partition	*/
	}

        SortParams second;      //Set up the second half of the SortParams structure for next quickSort() call
	second.array = array; 
	second.left = i; 
	second.right = right;

	if(currentThreads < maximumThreads) {  //If havnt made maximumThreads or maximumThreads arn't being used
		if((error = pthread_mutex_lock(&threadMutex)) != 0) {    //Lock mutex
			fprintf(stderr,"Error locking mutex, error = %d\n", error);
		}
		if(DEBUG) { printf("Creating thread for right side\n"); }
		if((error = pthread_create(&rightThread, NULL, (void *)quickSort, &second)) != 0) {  //Create thread for right half
			fprintf(stderr,"Error creating rightThread, error = %d\n", error);
		}
		currentThreads++;                    //Increment total threads
		rTCheck = 1;		//Set flag to wait to join
		if((error = pthread_mutex_unlock(&threadMutex) != 0)) {  //Unlock mutex
			fprintf(stderr,"Error unlocking mutex, error = %d\n", error);
		}
	}
	else {  //Otherwise quickSort() with the same thread
        	quickSort(&second);                 /* sort the right partition */
	}

	if(lTCheck == 1) {  //If a thread was created for first half, wait for it to terminate
		if(DEBUG) { printf("left waiting to join\n"); }
		if((error = pthread_join(leftThread, NULL)) != 0) {
			fprintf(stderr,"Error joining, error = %d\n", error);
		}
	}

	if(rTCheck == 1) {  //If a thread was created for second half, wait for it to terminate
		if(DEBUG) { printf("right waiting to join\n"); }
		if((error = pthread_join(rightThread, NULL)) != 0) {
			fprintf(stderr,"Error joining, error = %d\n", error);
		}
	}
				
    } else insertSort(array,i,j);           /* for a small range use insert sort */
}

/* user interface routine to set the number of threads sortT is permitted to use */

void setSortThreads(int count) {
    maximumThreads = count;
}

/* user callable sort procedure, sorts array of count strings, beginning at address array */

void sortThreaded(char** array, unsigned int count) {
    SortParams parameters;
    parameters.array = array; parameters.left = 0; parameters.right = count - 1;
    quickSort(&parameters);
}



//---------------------------------------QSORT IMPLEMENTATION-----------------------------------------

int compare (const void *a, const void *b ) {
    const char *pa = *(const char**)a;
    const char *pb = *(const char**)b;

    return strcmp(pa,pb);
}

//-------------------------------------FIND TOTAL NUMBER OF WORDS--------------------------------

int main(int argc, char **argv) {
	const char *filename = "poem.txt";
	char c, str[20];
	int wordCount = 0, i = 0;
	clock_t start, end;
	double cpuTimeUsed, cpuTimeUsed2;
	
	if(argc != 2) {
		fprintf(stderr, "./sort numberOfThreads\n");
		exit(1);
	}
	int numT = atoi(argv[1]);	
	setSortThreads(numT);  //Set the number of threads to be used
	printf("Number of threads to be used: %d\n",numT);

	FILE *fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr,"file open error\n");
		exit(1);
	}

	//Count the number of words in the file
	while( (c = fgetc(fp)) != EOF) {
		if(c == ' ' || c == '\n' || c == '.' || c == ':' || c == '' || c == ',' || c == '[' || c == ']' || c == ';' || c == '?' || c == '!') {
			str[i] = '\0';
			if(str[0] == '\0') {
				continue;
			}
			else {
				++wordCount;
				//printf("%s %d\n",str,wordCount);
				memset(str, 0, sizeof(str));
				i = 0;
			}
		}
		else {
			str[i] = c;
			i++;
		}
	}
	fclose(fp);

//--------------------------------MALLOC CHAR **ARRAY AND INSERT WORDS----------------------------
//
	//Malloc a char ** array the size of the total wordCount for quicksort
	char **wordArray = (char **) malloc(wordCount * sizeof(char *));
	for(int i = 0; i < wordCount; i++) {
		//Each char * (word) in the array can be a maximum of 20 char in length
		wordArray[i] = (char *) malloc(20 * sizeof(char));
	}

	//Malloc a char ** array2 the size of the total wordCount for qsort()
	char **wordArray2 = (char **) malloc(wordCount * sizeof(char *));
        for(int i = 0; i < wordCount; i++) {
                //Each char * (word) in the array can be a maximum of 20 char in length
                wordArray2[i] = (char *) malloc(20 * sizeof(char));
        }

	i = 0;  //Keep track of characters in single word
	int j = 0; //Keep track of where to insert into **wordArray

	fp = fopen(filename, "r");
	if(fp == NULL) {
                fprintf(stderr,"file open error\n");
                exit(1);
        }

	while( (c = fgetc(fp)) != EOF) {
        	if(c == ' ' || c == '\n' || c == '.' || c == ':' || c == '' || c == ',' || c == '[' || c == ']' || c == ';' || c == '?' || c == '!') {                
			str[i] = '\0';
                        if(str[0] == '\0') {
                                continue;
                        }
                        else {
				for(int k = 0; k < sizeof(str); k++) { //sizeof to include \0
					wordArray[j][k] = str[k];
					wordArray2[j][k] = str[k];
				}
				j++;
                                memset(str, 0, sizeof(str));
				str[0] == '\0';
                                i = 0;
                        }
                }
                else {
                        str[i] = c;
                        i++;
                }
        }
	fclose(fp);

	//printf("There are a total of %d words\n",wordCount);
	//sleep(5);
	//for(int i = 0; i < wordCount; i++) {
	//	printf("%s --- inserted into wordArray[%d]\n",wordArray[i],i);
	//}	
	//sleep(5);
//-------------------------------------TIME TO SORT-------------------------------------
	start = clock();
	qsort(wordArray,wordCount,sizeof(char *),compare);
	end = clock();
	cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	
	start = clock();
	sortThreaded(wordArray2, wordCount);
	end = clock();
	cpuTimeUsed2 = ((double) (end - start)) / CLOCKS_PER_SEC;

	//sleep(5);
	//for(int i = 0; i < wordCount; i++) {
        //        printf("%s --- sorted wordArray[%d]\n",wordArray[i],i);
        //}
	//sleep(5);
	//for(int i = 0; i < wordCount; i++) {
        //        printf("%s --- sorted wordArray2[%d]\n",wordArray2[i],i);
        //}

	printf("Benchmark qsort: %f\n",cpuTimeUsed);
	printf("Optimized sortThreaded quicksort: %f\n",cpuTimeUsed2);

	//free memory for char** array
	for(int i = 0; i < wordCount; i++) {
		free(wordArray[i]);
	}
	free(wordArray);

	//free memory for char** array2
        for(int i = 0; i < wordCount; i++) {
                free(wordArray2[i]);
        }
        free(wordArray2);
}

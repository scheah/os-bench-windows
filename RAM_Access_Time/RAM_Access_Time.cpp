// RAM_Access_Time.cpp : Defines the entry point for the console application.
/* 
RAM access time: Report latency for individual integer accesses to main memory and the L1 and L2 caches.
Present results as a graph with the x-axis as the log of the size of the memory region accessed, and the y-axis as the average latency.
Note that the lmbench paper is a good reference for this experiment.
In terms of the lmbench paper, measure the "back-to-back-load" latency and report your results in a graph similar to Fig. 1 in the paper.
You should not need to use information about the machine or the size of the L1, L2, etc., caches when implementing the experiment;
the experiment will reveal these sizes. In your graph, label the places that indicate the different hardware regimes (L1 to L2 transition, etc.).
*/

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

//num bytes
#define KB 1024
#define MB 1024 * KB

#define num_accesses	1000000	// 1 Mil Accesses
#define stride 			512	// in bytes

// MACRO For loop unrolling and mem. accessing
#define ONE 		p = (int **) *p;
#define FIVE    	ONE ONE ONE ONE ONE
#define TEN 		FIVE FIVE
#define FIFTY   	TEN TEN TEN TEN TEN
#define HUNDRED 	FIFTY FIFTY
#define FIVEHUNDRED HUNDRED HUNDRED HUNDRED HUNDRED HUNDRED
#define THOUSAND 	FIVEHUNDRED FIVEHUNDRED 

// For timing
static LARGE_INTEGER freq;
static LARGE_INTEGER start_time_stamp;
static LARGE_INTEGER end_time_stamp;

volatile int use_result_dummy;

// For use in fooling the compiler that the pointer is needed for a data operation
void use_pointer(void *result) {
	use_result_dummy += (int)result;
}

__int64 time_elapsed() { //in nanoseconds
	LARGE_INTEGER elapsed_time;
	elapsed_time.QuadPart = (end_time_stamp.QuadPart - start_time_stamp.QuadPart);
	elapsed_time.QuadPart *= (LONGLONG)1000000000.0;
	elapsed_time.QuadPart /= freq.QuadPart;
	return elapsed_time.QuadPart;
}

// return time elapsed for striding through members of array of specified size
__int64 time_ram_access(int array_size) {
	unsigned int i = 0; // use in looping
	unsigned int num_elements = array_size / sizeof(int);
	int *arr = (int*)malloc(sizeof(int) * num_elements);
	//set up data path to avoid prefetching, random accessses 
	//Key is linked list with nodes allocated stride bytes apart in a contiguous memory space
	int ** head = (int**)arr;
	int ** p = head; //iterator
	for (i = 0; i + stride / sizeof(int) < num_elements; i += stride / sizeof(int)) {
		*p = &arr[i + stride / sizeof(int)];
		p = p + stride / sizeof(p);
	}
	*p = (int *)head; // tail points to head
	p = head;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start_time_stamp);
	for (i = 0; i < num_accesses / 1000; i++) { // minus loop overhead in the future
		THOUSAND
	}
	QueryPerformanceCounter(&end_time_stamp);
	use_pointer((void*)p); //to prevent compiler thinking this is a dead pointer
	free(arr);
	return time_elapsed() / num_accesses;
}

void write_results(char * name, int num_sizes, int array_sizes[], __int64 latency_results[]) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, name, "w");
	if (err) {
		printf("The file %s was not opened\n", name);
		return;
	}
	fprintf(fp, "array size(bytes) \t latency(ns)\n");  //print results to file
	for (int i = 0; i < num_sizes; i++) {
		fprintf(fp, "%*d\t\t%*I64u\n", 12, array_sizes[i], 6, latency_results[i]);  //print results to file
	}
	fclose(fp);
}

int _tmain(int argc, _TCHAR* argv[])
{
	int array_sizes[] = { //graph points are powers of 2
		1 * KB, 2 * KB, 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB, 256 * KB, 512 * KB,
		1 * MB, 2 * MB, 4 * MB, 8 * MB, 16 * MB
	};
	const int num_sizes = sizeof(array_sizes) / sizeof(int);
	__int64 latency_results[num_sizes]; // results will be stored here
	int i;
	for (i = 0; i < num_sizes; i++) {
		latency_results[i] = time_ram_access(array_sizes[i]);
	}
	write_results((char*)"RAM_Access_Time_Results.txt", num_sizes, array_sizes, latency_results);
	return 0;
}


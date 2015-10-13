// CPU_System_Overhead.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

#define NUM_TESTS		1000			// take average time

// For timing
static LARGE_INTEGER freq;
static LARGE_INTEGER start_time_stamp;
static LARGE_INTEGER end_time_stamp;

__int64 time_elapsed() { //in nanoseconds
	LARGE_INTEGER elapsed_time;
	elapsed_time.QuadPart = (end_time_stamp.QuadPart - start_time_stamp.QuadPart);
	elapsed_time.QuadPart *= (LONGLONG)1000000000.0;
	elapsed_time.QuadPart /= freq.QuadPart;
	return elapsed_time.QuadPart;
}

double measurement_overhead() {
	__int64 iTotalTime = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		// do nothing
		QueryPerformanceCounter(&end_time_stamp);
		iTotalTime += time_elapsed();
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return dTotalTime / NUM_TESTS;
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	return 0;
}


double system_overhead()
{
	// NOTE we might have to consider the first running of a system call
	__int64 iTotalTime = 0;
	// Create the thread
	DWORD tid = 0;
	HANDLE hid = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		MyThreadFunction,       // thread function name
		NULL,		            // argument to thread function 
		CREATE_SUSPENDED,       // create, BUT do not run! 
		&tid);					// returns the thread identifier
	for (int i = 0; i < NUM_TESTS; i++) {
		// Start timing
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		GetThreadId(hid);
		QueryPerformanceCounter(&end_time_stamp);
		iTotalTime += time_elapsed();
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return (dTotalTime / NUM_TESTS) - measurement_overhead();
}

void write_results(char * fileName, double dSystemOverhead) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, fileName, "w");
	if (err) {
		printf("The file %s was not opened\n", fileName);
		return;
	}
	fprintf(fp, "Overhead \t\t\t Time (ns)\n");  //print results to file
	fprintf(fp, "%-*s\t\t%*f\n", 16, "System", 10, dSystemOverhead);
	fclose(fp);
}


int _tmain(int argc, _TCHAR* argv[])
{
	double dSystemOverhead = system_overhead();
	write_results("CPU_System_Overhead_Results.txt", dSystemOverhead);
	return 0;
}


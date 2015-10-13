// CPU_Task_Creation.cpp : Defines the entry point for the console application.
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

// ---------------------------------------------------------------------------
// Test thread function.
// ---------------------------------------------------------------------------
DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	return 0;
}

// ---------------------------------------------------------------------------
// Calculates the thread creation cycles
// ---------------------------------------------------------------------------
double threadTest()
{
	__int64 iTotalTime = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		// Start timing
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		// Create the thread
		DWORD tid = 0;
		HANDLE hid = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MyThreadFunction,       // thread function name
			NULL,		            // argument to thread function 
			CREATE_SUSPENDED,       // create, BUT do not run! 
			&tid);					// returns the thread identifier
		QueryPerformanceCounter(&end_time_stamp);
		CloseHandle(hid);
		iTotalTime += time_elapsed();
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return (dTotalTime / NUM_TESTS) - measurement_overhead();
}

// ---------------------------------------------------------------------------
// Calculates the process creation cycles
// ---------------------------------------------------------------------------
double processTest()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	__int64 iTotalTime = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		// Start timing
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		// Create process and thread
		if (!CreateProcess(L"C:\\Windows\\notepad.exe",
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_SUSPENDED,
			NULL,
			NULL,
			&si,
			&pi)
			)
		{
			return -1;
		}
		// Get the end time
		QueryPerformanceCounter(&end_time_stamp);
		// Terminate process and thread
		TerminateProcess(pi.hProcess, NULL);
		CloseHandle(pi.hThread);
		iTotalTime += time_elapsed();
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return (dTotalTime / NUM_TESTS) - measurement_overhead();
}

void write_results(char * fileName, double dThreadCreationTime, double dProcessCreationTime) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, fileName, "w");
	if (err) {
		printf("The file %s was not opened\n", fileName);
		return;
	}
	fprintf(fp, "Task Creation \t\t\t Creation Time (ns)\n");  //print results to file
	fprintf(fp, "%-*s\t\t%*f\n", 16, "Thread", 10, dThreadCreationTime);
	fprintf(fp, "%-*s\t\t%*f\n", 16, "Process", 10, dProcessCreationTime);
	fclose(fp);
}

int _tmain(int argc, _TCHAR* argv[])
{
	double dThreadCreationTime;
	double dProcessCreationTime;
	dThreadCreationTime = threadTest();
	dProcessCreationTime = processTest();
	write_results("CPU_Task_Creation_Results.txt", dThreadCreationTime, dProcessCreationTime);
	return 0;
}


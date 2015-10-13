// CPU_Context_Switch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

#define NUM_TESTS		50			// take average time to create process/threads

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

__int64 time_elapsed(__int64 currentProcessTime, __int64 childProcessTime) { // in nanoseconds
	LARGE_INTEGER elapsed_time;
	elapsed_time.QuadPart = childProcessTime - currentProcessTime;
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
	QueryPerformanceCounter(&end_time_stamp);
	return 0;
}


double thread_context_switch()
{
	// NOTE we might have to consider the first running of a system call
	__int64 iTotalTime = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		DWORD tid = 0;
		HANDLE hid = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MyThreadFunction,       // thread function name
			NULL,		            // argument to thread function 
			CREATE_SUSPENDED,       // create, BUT do not run! 
			&tid);					// returns the thread identifier
		SetThreadPriority(hid, 10); // set high priority for scheduler
		// Start timing
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		ResumeThread(hid);
		WaitForSingleObject(hid, INFINITE);
		iTotalTime += time_elapsed();
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return (dTotalTime / NUM_TESTS) - measurement_overhead();
}

double process_context_switch()
{
	
	// create a child->parent named pipe
	/*HANDLE pipe = CreateNamedPipe(
		L"\\\\.\\pipe\\mypipe",			// name of the pipe
		PIPE_ACCESS_INBOUND,			// receive only
		PIPE_TYPE_BYTE,					// send data as a byte stream
		1,								// only one instance
		0, 0, 0, NULL);					// default
	if (pipe == INVALID_HANDLE_VALUE) return -1; //error*/
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	__int64 iTotalTime = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		// Start timing
		// Create process and thread
		LPTSTR szCmdline = _tcsdup(TEXT("CPU_Dummy_Child_Process.exe"));
		if (!CreateProcess(NULL,
			szCmdline,
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
		
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		ResumeThread(pi.hThread);
		WaitForSingleObject(pi.hThread, 2000);
		LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mypipe");
		// Try to open a named pipe; wait for it, if necessary. 
		HANDLE hPipe;
		while (1)
		{
			hPipe = CreateFile(
				lpszPipename,   // pipe name 
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				0,              // default attributes 
				NULL);          // no template file 

			// Break if the pipe handle is valid. 

			if (hPipe != INVALID_HANDLE_VALUE)
				break;

			// Exit if an error other than ERROR_PIPE_BUSY occurs. 

			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
				//return -1;
			}

			// All pipe instances are busy, so wait for 20 seconds. 

			if (!WaitNamedPipe(lpszPipename, 20000))
			{
				printf("Could not open pipe: 20 second wait timed out.");
				//return -1;
			}
		}
		//HANDLE hPipe = CreateFile(L"\\\\.\\pipe\\pipename", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		//WaitForSingleObject(pi.hProcess, INFINITE);
		
		//BOOL result = ConnectNamedPipe(pipe, NULL);
		
		//_atoi64
		// Get the end time
		char buffer[21];
		DWORD read = 0;
		while (1) {
			if (!ReadFile(
				hPipe,
				buffer,           // read data
				sizeof(buffer) - 1, // max length (leave room for terminator)
				&read,            // bytes actually read
				NULL))
				continue;
			else
				break; //success!
		}
		__int64 procEnd = _atoi64(buffer);
		iTotalTime += time_elapsed(start_time_stamp.QuadPart, procEnd);
#ifdef _DEBUG
		printf("total time is now %I64d\n", iTotalTime);
		// Terminate process and thread
#endif
		TerminateProcess(pi.hProcess, NULL);
		CloseHandle(pi.hThread);
		CloseHandle(hPipe);
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return (dTotalTime / NUM_TESTS) - measurement_overhead();
}


void write_results(char * fileName, double dThreadContextSwitch, double dProcessContextSwitch) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, fileName, "w");
	if (err) {
		printf("The file %s was not opened\n", fileName);
		return;
	}
	fprintf(fp, "Context Switch \t\t\t Measured Time (ns)\n");  //print results to file
	fprintf(fp, "%-*s\t\t%*f\n", 16, "Thread", 10, dThreadContextSwitch);
	fprintf(fp, "%-*s\t\t%*f\n", 16, "Process", 10, dProcessContextSwitch);
	fclose(fp);
}

int _tmain(int argc, _TCHAR* argv[])
{
	double dThreadContextSwitch = thread_context_switch();
	double dProcContextSwitch = process_context_switch();
	write_results("CPU_Context_Switch_Results.txt", dThreadContextSwitch, dProcContextSwitch);
	return 0;
}


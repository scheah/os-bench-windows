// CPU_Dummy_Child_Process.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

static LARGE_INTEGER freq;
static LARGE_INTEGER end_time_stamp;

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE pipe = CreateNamedPipe(
		L"\\\\.\\pipe\\mypipe",			// name of the pipe
		PIPE_ACCESS_OUTBOUND | FILE_FLAG_WRITE_THROUGH,			// 
		PIPE_TYPE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
		512, 512, 1000, NULL);					// default
	if (pipe == INVALID_HANDLE_VALUE) return -1; //error
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&end_time_stamp);
	/*HANDLE pipe = CreateFile(
		L"\\\\.\\pipe\\pipename",		// name of the pipe
		GENERIC_WRITE,				//
		0, NULL,					//
		OPEN_EXISTING,				// opens existing pipe
		0, NULL);					//
		if (pipe == INVALID_HANDLE_VALUE) return -1; //error*/
	char line[21];
	_i64toa_s(end_time_stamp.QuadPart, line, 21, 10);
	DWORD written = 0;
	int test = strlen(line);
	while (1) {
		if (WriteFile(
			pipe,
			line,         // sent data
			strlen(line), // data length
			&written,     // bytes actually written
			NULL))
			break;

	}

	DisconnectNamedPipe(pipe);
	CloseHandle(pipe);
	
	return 0;
}


// FileContentionMain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <sstream>

// Global vars
static const wchar_t* FILE_DIRECTORY = L"C:\\CSE221\\Contention";

int _tmain(int argc, _TCHAR* argv[])
{
	static const int PROCESS_CNT = 2;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	for (int i = 0; i<PROCESS_CNT; i++)
	{
		std::wstringstream processNameStrm;
		processNameStrm << FILE_DIRECTORY << L"\\ContentionProcess " << i + 1;

		// Create process and thread
		if (CreateProcess(NULL,
			(LPWSTR)processNameStrm.str().c_str(),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi) == FALSE)
		{
			return 1;
		}
	}
	while (1){
	}
	return 0;
}


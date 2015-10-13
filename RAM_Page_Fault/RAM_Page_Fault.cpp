// Measures the RAM read and write bandwidth
//

#include "stdafx.h"
#include "windows.h"
#include "RAM_Page_Fault.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>             // For exit

#define PAGELIMIT 100            // Number of pages to ask for

LPTSTR lpNxtPage;               // Address of the next page to ask for
DWORD dwPages = 0;              // Count of pages gotten so far
DWORD dwPageSize;               // Page size on this computer

static DWORD failCnt = 0;				// Counter for tracking faults
static LONGLONG timeAvg = 0;			// Maintain an average in ns

// ---------------------------------------------------------------------------
// PageFaultExceptionFilter
// ---------------------------------------------------------------------------
INT PageFaultExceptionFilter(DWORD dwCode)
{
	LARGE_INTEGER startTimeStamp;	// Start time stamp
	LARGE_INTEGER endTimeStamp;		// End time stamp
	LARGE_INTEGER frequency;		// Frequency

	LPVOID lpvResult;

	// If the exception is not a page fault, exit.

	if (dwCode != EXCEPTION_ACCESS_VIOLATION)
	{
		printf("Exception code = %d.\n", dwCode);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	printf("Exception is a page fault.\n");

	// If the reserved pages are used up, exit.
	if (dwPages >= PAGELIMIT)
	{
		printf("Exception: out of pages.\n");
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// Otherwise, time commiting another page.
	QueryPerformanceCounter(&startTimeStamp);
	lpvResult = VirtualAlloc(
		(LPVOID)lpNxtPage, // Next page to commit
		dwPageSize,         // Page size, in bytes
		MEM_COMMIT,         // Allocate a committed page
		PAGE_READWRITE);    // Read/write access
	QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);
	if (lpvResult == NULL)
	{
		printf("VirtualAlloc failed.\n");
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
		printf("Allocating another page.\n");
	}

	// Increment the page count, and advance lpNxtPage to the next page.
	dwPages++;
	lpNxtPage = (LPTSTR)((PCHAR)lpNxtPage + dwPageSize);

	// Update timing
	timeAvg += (endTimeStamp.QuadPart - startTimeStamp.QuadPart) * 1000000000 / frequency.QuadPart;
	failCnt++;

	// Continue execution where the page fault occurred.
	return EXCEPTION_CONTINUE_EXECUTION;
}

// ---------------------------------------------------------------------------
// GetRAM_ReadBandwidth
// ---------------------------------------------------------------------------
LONGLONG GetRAM_Page_Fault()
{
	LPVOID lpvBase;               // Base address of the test memory
	LPTSTR lpPtr;				  // Generic character pointer
	BOOL bSuccess;                // Flag
	DWORD i;                      // Generic counter
	SYSTEM_INFO sSysInfo;         // Useful information about the system

	GetSystemInfo(&sSysInfo);     // Initialize the structure.

	// Get the page size
	printf("This computer has page size %d.\n", sSysInfo.dwPageSize);
	dwPageSize = sSysInfo.dwPageSize;

	// Reserve pages in the virtual address space of the process.
	lpvBase = VirtualAlloc(
		NULL,                 // System selects address
		PAGELIMIT*dwPageSize, // Size of allocation
		MEM_RESERVE,          // Allocate reserved pages
		PAGE_NOACCESS);       // Protection = no access
	if (lpvBase == NULL)
		return timeAvg / failCnt;

	lpPtr = (LPTSTR)lpvBase;
	lpNxtPage = (LPTSTR)lpvBase;

	// Use structured exception handling when accessing the pages.
	// If a page fault occurs, the exception filter is executed to
	// commit another page from the reserved block of pages.

	for (i = 0; i < PAGELIMIT*(dwPageSize - 1); i++)
	{
		__try
		{
			// Write to memory.

			lpPtr[i] = 'a';
		}

		// If there's a page fault, commit another page and try again.

		__except (PageFaultExceptionFilter(GetExceptionCode()))
		{

			// This code is executed only if the filter function
			// is unsuccessful in committing the next page.

			_tprintf(TEXT("Exiting process.\n"));

			if (failCnt > 0)
				printf("Current average pagefault time: %d", timeAvg / failCnt);

			ExitProcess(GetLastError());

		}

	}

	// Release the block of pages when you are finished using them.

	bSuccess = VirtualFree(
		lpvBase,       // Base address of block
		0,             // Bytes of committed pages
		MEM_RELEASE);  // Decommit the pages

	_tprintf(TEXT("Release %s.\n"), bSuccess ? TEXT("succeeded") : TEXT("failed"));

	return timeAvg / failCnt;
}

// Application entry point
int _tmain(int argc, _TCHAR* argv[])
{
	printf("Page Fault (ns): %I64d \n", GetRAM_Page_Fault());
	return 0;
}
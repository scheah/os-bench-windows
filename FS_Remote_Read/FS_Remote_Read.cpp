// FileReadTimeMeasurement.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include <sstream>
#include <fstream>

HANDLE createTestFile(INT64 size);
ULONGLONG readFileTime(HANDLE file, DWORD bytesRequested);
ULONGLONG readRandomFileTime(HANDLE file, DWORD bytesRequested);

// Global vars
static const DWORD _dwPageSize = 4096;
static const wchar_t* FILE_DIRECTORY = L"U:\\CSE221";

// Application entry point
int _tmain(int argc, _TCHAR* argv[])
{
	static const int SAMPLE_CNT = 12;
	INT64 fileSize[SAMPLE_CNT];
	HANDLE fileHandle[SAMPLE_CNT];
	ULONGLONG fileReadTime[SAMPLE_CNT];
	ULONGLONG fileRandomReadTime[SAMPLE_CNT];

	// File size in bytes as a factor of pages
	fileSize[0] = _dwPageSize;
	for (int i = 1; i<SAMPLE_CNT; i++)
	{
		fileSize[i] = fileSize[i - 1] * 2;
	}

	// Create the files
	printf("Creating files...\n");
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		fileHandle[i] = createTestFile(fileSize[i]);
	}

	// Get the read time for the files
	printf("Reading files...\n");
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		fileReadTime[i] = readFileTime(fileHandle[i], (DWORD)fileSize[i]);
	}
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		fileRandomReadTime[i] = readRandomFileTime(fileHandle[i], (DWORD)fileSize[i]);
	}

	// Output the results
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		printf("%I64u\n", fileReadTime[i]);
	}
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		printf("%I64u\n", fileRandomReadTime[i]);
	}

	while (1) {
	}

	return 0;
}

// Creates a file of a specified size. Specified size assumed even.
HANDLE createTestFile(INT64 size)
{
	DWORD rc;
	BOOL err;
	HANDLE fileHandle;

	// Check size is even.
	if (size % 2 != 0)
		return INVALID_HANDLE_VALUE;

	// Create a directory for the files
	rc = GetFileAttributes(FILE_DIRECTORY);
	if (rc == INVALID_FILE_ATTRIBUTES)
	{
		err = CreateDirectory(FILE_DIRECTORY, NULL);
		if (err == FALSE)
			return INVALID_HANDLE_VALUE;
	}

	//Check for existing files.
	std::wstringstream testFileNameStrm;
	testFileNameStrm << FILE_DIRECTORY << L"\\Test_File_SIZE_" << size;

	fileHandle = CreateFile((LPTSTR)testFileNameStrm.str().c_str(), // file name 
		GENERIC_READ,         // open for reading
		0,                    // do not share 
		NULL,                 // default security 
		OPEN_EXISTING,        // check for existance
		FILE_FLAG_NO_BUFFERING,// do not use file system cache
		NULL);                // no template 
	return fileHandle;
}

// Returns the time to read the file
ULONGLONG readFileTime(HANDLE file, DWORD bytesRequested)
{
	LARGE_INTEGER startTimeStamp;
	LARGE_INTEGER endTimeStamp;
	LARGE_INTEGER frequency;

	// Check for input validity
	if (file == INVALID_HANDLE_VALUE || bytesRequested <= 0)
		return 0;

	// Allocate some memory to read the file
	LPVOID pArray = NULL;
	pArray = HeapAlloc(
		GetProcessHeap(),		// Current process handle
		NULL,					// Allocation flags
		bytesRequested			// Bytes
		);
	if (pArray == NULL){
		printf("Page allocation failed %d.\n", GetLastError());
		return 0;
	}

	// Get the start time
	QueryPerformanceCounter(&startTimeStamp);

	// Read in the file
	DWORD bytesRead = 0;
	BOOL rc = ReadFile(
		file,					// Handle to file
		pArray,					// Buffer begin
		bytesRequested,			// Bytes to read in
		&bytesRead,				// Bytes read back
		NULL					// Optional
		);
	if (rc == FALSE)
	{
		printf("Read error for bytes %d\n", bytesRequested);
		HeapFree(GetProcessHeap(), NULL, pArray);
		return 0;
	}

	// Get the end time
	QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);
	HeapFree(GetProcessHeap(), NULL, pArray);

	return ((ULONGLONG)endTimeStamp.QuadPart - (ULONGLONG)startTimeStamp.QuadPart) * 1000000U / frequency.QuadPart;
}

// Read the file bytes randomly
ULONGLONG readRandomFileTime(HANDLE file, DWORD bytesRequested)
{
	LARGE_INTEGER startTimeStamp;
	LARGE_INTEGER endTimeStamp;
	LARGE_INTEGER frequency;
	DWORD randomInt = 0;
	wchar_t wChar[_dwPageSize];
	DWORD blockCnt = bytesRequested / _dwPageSize;

	// Check for input validity
	if (file == INVALID_HANDLE_VALUE || bytesRequested <= 0)
		return 0;

	// Close the file handle. Using file stream.
	CloseHandle(file);

	// Read the file stream
	std::wstringstream testFileNameStrm;
	testFileNameStrm << FILE_DIRECTORY << L"\\Test_File_SIZE_" << bytesRequested;

	std::wifstream iFile;
	iFile.open(testFileNameStrm.str().c_str(), std::ifstream::in);

	// Get the start time
	QueryPerformanceCounter(&startTimeStamp);

	// Fetch random blocks from the file
	for (DWORD i = 0; i<blockCnt; i++)
	{
		randomInt = rand() % blockCnt;
		iFile.seekg(randomInt*_dwPageSize, std::ios::beg);
		iFile.get(&wChar[0], _dwPageSize);
	}

	// Get the end time
	QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);

	return ((ULONGLONG)endTimeStamp.QuadPart - (ULONGLONG)startTimeStamp.QuadPart) * 1000000U / frequency.QuadPart;
}
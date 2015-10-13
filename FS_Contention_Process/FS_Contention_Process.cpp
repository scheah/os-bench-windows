// Contention process creates files for itself to access and then tries to read from those files.

#include "stdafx.h"
#include "AccessTimeMovingAverage.h"
#include <sstream>
#include <fstream>

HANDLE createTestFile(int processId, int fileId);
void readFileTime(HANDLE file);

// Global vars
AccessTimeMovingAverage<50> _timeAvg;
static const DWORD _dwPageSize = 4096;
static const DWORD _fileSize = 2 * _dwPageSize;
static const DWORD _readBytes = _dwPageSize;
static const wchar_t* FILE_DIRECTORY = L"C:\\CSE221\\Contention";

int _tmain(int argc, _TCHAR* argv[])
{
	static const int SAMPLE_CNT = 100;
	HANDLE fileHandle[SAMPLE_CNT];

	int argInt = 0;
	int cnt = 0;
	while (++cnt < argc && argc != NULL){
		argInt += ((int)*argv[cnt] - (int)'0') * 10 * cnt;
	}

	// Create the files for this process to read
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		fileHandle[i] = createTestFile(argInt, i);
	}

	// Get the read time for the files
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		readFileTime(fileHandle[i]);
	}
	Sleep(5);

	// Output the results to a file
	std::wstringstream outputFileNameStrm;
	outputFileNameStrm << FILE_DIRECTORY << L"\\Results_" << argInt;
	std::wofstream file;
	file.open(outputFileNameStrm.str().c_str());
	file << "Average time (ns) " << _timeAvg.Average() << "\n";
	file.close();

	while (1) {
	}

	return 0;
}

// Create test files for the process to read
HANDLE createTestFile(int processId, int fileId)
{
	DWORD rc;
	BOOL err;
	HANDLE fileHandle;

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
	testFileNameStrm << FILE_DIRECTORY << L"\\Test_File_" << processId << L"_" << fileId;

	fileHandle = CreateFile((LPTSTR)testFileNameStrm.str().c_str(), // file name 
		GENERIC_READ,         // open for reading
		0,                    // do not share 
		NULL,                 // default security 
		OPEN_EXISTING,        // check for existance
		FILE_FLAG_NO_BUFFERING,// do not use file system cache
		NULL);                // no template 
	if (fileHandle != INVALID_HANDLE_VALUE)
		return fileHandle;

	// File does not exist. We need to create a file of the specified size
	std::wofstream file;
	file.open(testFileNameStrm.str().c_str());
	INT64 cnt = 0;
	INT64 maxWrt = _fileSize / 2;
	while (++cnt<maxWrt)
	{
		file << "aa";
	}
	file.close();

	// Try to get the handle again
	fileHandle = CreateFile((LPTSTR)testFileNameStrm.str().c_str(), // file name 
		GENERIC_READ,         // open for write 
		0,                    // do not share 
		NULL,                 // default security 
		OPEN_EXISTING,        // check for existance
		FILE_FLAG_NO_BUFFERING,// do not use file system cache
		NULL);                // no template 
	return fileHandle;
}

// Time reading the file with the AccessTimeMovingAverage object
void readFileTime(HANDLE file)
{
	// Check for input validity
	if (file == INVALID_HANDLE_VALUE)
		return;

	// Allocate some memory to read the file
	LPVOID pArray = NULL;
	pArray = HeapAlloc(
		GetProcessHeap(),		// Current process handle
		NULL,					// Allocation flags
		_readBytes				// Bytes
		);
	if (pArray == NULL){
		printf("Page allocation failed %d.\n", GetLastError());
		return;
	}

	// Get the start time
	_timeAvg.AddNewStartTimeStamp();

	// Read in the file
	DWORD bytesRead = 0;
	BOOL rc = ReadFile(
		file,					// Handle to file
		pArray,					// Buffer begin
		_readBytes,				// Bytes to read in
		&bytesRead,				// Bytes read back
		NULL					// Optional
		);
	if (rc == FALSE)
	{
		printf("Read error \n");
		HeapFree(GetProcessHeap(), NULL, pArray);
		return;
	}

	// Get the end time
	_timeAvg.AddNewEndTimeStamp();
	HeapFree(GetProcessHeap(), NULL, pArray);
}
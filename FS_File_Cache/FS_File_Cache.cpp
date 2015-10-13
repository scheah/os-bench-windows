// FileCacheSizeMeasurement.cpp : Defines the entry point for the console application.
//

// Follows the example given at https://msdn.microsoft.com/en-us/library/windows/desktop/aa373178(v=vs.85).aspx

#include "stdafx.h"
#include "windows.h"
#include <sstream>
#include <fstream>


// Contains the elements required to calculate a counter value.
typedef struct _rawdata
{
	DWORD CounterType;
	ULONGLONG Data;          // Raw counter data
}RAW_DATA, *PRAW_DATA;

#define INIT_OBJECT_BUFFER_SIZE 48928   // Initial buffer size to use when querying specific objects.
#define INIT_GLOBAL_BUFFER_SIZE 122880  // Initial buffer size to use when using "Global" to query all objects.
#define BUFFER_INCREMENT 16384          // Increment buffer size in 16K chunks.
#define MAX_INSTANCE_NAME_LEN      255  // Max length of an instance name.
#define MAX_FULL_INSTANCE_NAME_LEN 511  // Form is parentinstancename/instancename#nnn.

HANDLE createTestFile(DWORD size);
ULONGLONG readFileTime(HANDLE file, DWORD bytesRequested);
void readFileCacheSize();
void readFileCachePeakSize();
LPBYTE GetPerformanceData(LPWSTR pwszSource, DWORD dwInitialBufferSize);
BOOL GetCounterValues(DWORD dwObjectIndex, DWORD dwCounterIndex, LPWSTR pInstanceName, RAW_DATA* pRawData);
BOOL GetValue(PERF_OBJECT_TYPE* pObject, PERF_COUNTER_DEFINITION* pCounter, PERF_COUNTER_BLOCK* pCounterDataBlock, PRAW_DATA pRawData);
PERF_COUNTER_BLOCK* GetCounterBlock(PERF_OBJECT_TYPE* pObject, LPWSTR pInstanceName);
PERF_COUNTER_DEFINITION* GetCounter(PERF_OBJECT_TYPE* pObject, DWORD dwCounterToFind);
PERF_OBJECT_TYPE* GetObject(DWORD dwObjectToFind);

//Global variables
LPBYTE g_pPerfDataHead = NULL;   // Head of the performance data.
static const DWORD _dwPageSize = 4096;
static const wchar_t* FILE_DIRECTORY = L"C:\\CSE221";

// Main application entry poing
int _tmain(int argc, _TCHAR* argv[])
{
	static const int SAMPLE_CNT = 20;
	DWORD fileSize[SAMPLE_CNT];
	HANDLE fileHandle[SAMPLE_CNT];
	ULONGLONG fileReadTime[SAMPLE_CNT];

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
		FlushFileBuffers(fileHandle[i]);
		fileReadTime[i] = readFileTime(fileHandle[i], fileSize[i]);
		readFileCacheSize();
	}

	// Output the results
	for (int i = 0; i<SAMPLE_CNT; i++)
	{
		printf("%I64u\n", fileReadTime[i]);
	}
	void readFileCachePeakSize();

	while (1)
	{
	};

	return 0;
}

// Creates a file of a specified size. Specified size assumed even.
HANDLE createTestFile(DWORD size)
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
		GENERIC_READ | GENERIC_WRITE, // open for reading and writing. Need to write access to flush file cache.
		0,                    // do not share 
		NULL,                 // default security 
		OPEN_EXISTING,        // check for existance
		FILE_ATTRIBUTE_NORMAL,// use file system cache
		NULL);                // no template 
	if (fileHandle != INVALID_HANDLE_VALUE)
		return fileHandle;

	// File does not exist. We need to create a file of the specified size
	std::wofstream file;
	file.open(testFileNameStrm.str().c_str());
	DWORD cnt = 0;
	DWORD maxWrt = size / 2;
	while (++cnt<maxWrt)
	{
		file << "aa";
	}
	file.close();

	// Try to get the handle again
	fileHandle = CreateFile((LPTSTR)testFileNameStrm.str().c_str(), // file name 
		GENERIC_READ | GENERIC_WRITE,         // open for reading and writing. Need to write access to flush file cache.
		0,                    // do not share 
		NULL,                 // default security 
		OPEN_EXISTING,        // check for existance
		FILE_ATTRIBUTE_NORMAL,// use file system cache
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

// Retrieve the OS File Cache Size setting
void readFileCacheSize()
{
	RAW_DATA Sample1;
	BOOL fSuccess = FALSE;
	static const int SAMPLE_CNT = 3;
	ULONGLONG dataAvg = 0L;

	// Display five data points for the counter.
	for (DWORD i = 0; i < SAMPLE_CNT; i++)
	{
		Sleep(1000);  // Wait one second before taking the next sample

		// Retrieve the object data again and get the next sample.
		// Object 4 is Memory
		g_pPerfDataHead = (LPBYTE)GetPerformanceData(L"4", INIT_OBJECT_BUFFER_SIZE);
		if (NULL == g_pPerfDataHead)
		{
			wprintf(L"GetPerformanceData in loop failed.\n");
			goto cleanup;
		}

		// Counter 818 is Cache Bytes. We are retrieving the counter for Memory/Cache Bytes.
		fSuccess = GetCounterValues(4, 818, L"0", &Sample1);
		if (FALSE == fSuccess)
		{
			wprintf(L"GetCounterValues failed.\n");
			goto cleanup;
		}
		dataAvg += Sample1.Data;
		wprintf(L"File sys cache bytes %I64u.\n", Sample1.Data);
	}
	wprintf(L"Average value is %I64u.\n", dataAvg / SAMPLE_CNT);

cleanup:

	if (g_pPerfDataHead)
		free(g_pPerfDataHead);
}

// Retrieve the OS Peak File Cache Size Setting
void readFileCachePeakSize()
{
	RAW_DATA Sample1;
	BOOL fSuccess = FALSE;

	// Retrieve the object data again and get the next sample.
	// Object 4 is Memory
	g_pPerfDataHead = (LPBYTE)GetPerformanceData(L"4", INIT_OBJECT_BUFFER_SIZE);
	if (NULL == g_pPerfDataHead)
	{
		wprintf(L"GetPerformanceData in loop failed.\n");
		goto cleanup;
	}

	// Counter 820 is Cache Bytes Peak.
	fSuccess = GetCounterValues(4, 820, L"0", &Sample1);
	if (FALSE == fSuccess)
	{
		wprintf(L"GetCounterValues failed.\n");
		goto cleanup;
	}
	wprintf(L"File sys cache bytes peak %I64u.\n", Sample1.Data);

cleanup:

	if (g_pPerfDataHead)
		free(g_pPerfDataHead);
}

// Retrieve a buffer that contains the specified performance data.
LPBYTE GetPerformanceData(LPWSTR pwszSource, DWORD dwInitialBufferSize)
{
	LPBYTE pBuffer = NULL;
	DWORD dwBufferSize = 0;        //Size of buffer, used to increment buffer size
	DWORD dwSize = 0;              //Size of buffer, used when calling RegQueryValueEx
	LPBYTE pTemp = NULL;           //Temp variable for realloc() in case it fails
	LONG status = ERROR_SUCCESS;

	dwBufferSize = dwSize = dwInitialBufferSize;

	pBuffer = (LPBYTE)malloc(dwBufferSize);
	if (pBuffer)
	{
		while (ERROR_MORE_DATA == (status = RegQueryValueEx(HKEY_PERFORMANCE_DATA, pwszSource, NULL, NULL, pBuffer, &dwSize)))
		{
			//Contents of dwSize is unpredictable if RegQueryValueEx fails, which is why
			//you need to increment dwBufferSize and use it to set dwSize.
			dwBufferSize += BUFFER_INCREMENT;

			pTemp = (LPBYTE)realloc(pBuffer, dwBufferSize);
			if (pTemp)
			{
				pBuffer = pTemp;
				dwSize = dwBufferSize;
			}
			else
			{
				wprintf(L"Reallocation error.\n");
				free(pBuffer);
				pBuffer = NULL;
				goto cleanup;
			}
		}

		if (ERROR_SUCCESS != status)
		{
			wprintf(L"RegQueryValueEx failed with 0x%x.\n", status);
			free(pBuffer);
			pBuffer = NULL;
		}
	}
	else
	{
		wprintf(L"malloc failed to allocate initial memory request.\n");
	}

cleanup:

	RegCloseKey(HKEY_PERFORMANCE_DATA);

	return pBuffer;
}


// Use the object index to find the object in the performance data. Then, use the
// counter index to find the counter definition. The definition contains the offset
// to the counter data in the counter block. The location of the counter block 
// depends on whether the counter is a single instance counter or multiple instance counter.
// After finding the counter block, retrieve the counter data.
BOOL GetCounterValues(DWORD dwObjectIndex, DWORD dwCounterIndex, LPWSTR pInstanceName, RAW_DATA* pRawData)
{
	PERF_OBJECT_TYPE* pObject = NULL;
	PERF_COUNTER_DEFINITION* pCounter = NULL;
	PERF_COUNTER_BLOCK* pCounterDataBlock = NULL;
	BOOL fSuccess = FALSE;

	pObject = GetObject(dwObjectIndex);
	if (NULL == pObject)
	{
		wprintf(L"Object  %d not found.\n", dwObjectIndex);
		goto cleanup;
	}

	pCounter = GetCounter(pObject, dwCounterIndex);
	if (NULL == pCounter)
	{
		wprintf(L"Counter %d not found.\n", dwCounterIndex);
		goto cleanup;
	}

	// Retrieve a pointer to the beginning of the counter data block. The counter data
	// block contains all the counter data for the object.
	pCounterDataBlock = GetCounterBlock(pObject, pInstanceName);
	if (NULL == pCounterDataBlock)
	{
		wprintf(L"Instance %s not found.\n", pInstanceName);
		goto cleanup;
	}

	ZeroMemory(pRawData, sizeof(RAW_DATA));
	pRawData->CounterType = pCounter->CounterType;
	fSuccess = GetValue(pObject, pCounter, pCounterDataBlock, pRawData);

cleanup:

	return fSuccess;
}


// Use the object index to find the object in the performance data.
PERF_OBJECT_TYPE* GetObject(DWORD dwObjectToFind)
{
	LPBYTE pObject = g_pPerfDataHead + ((PERF_DATA_BLOCK*)g_pPerfDataHead)->HeaderLength;
	DWORD dwNumberOfObjects = ((PERF_DATA_BLOCK*)g_pPerfDataHead)->NumObjectTypes;
	BOOL fFoundObject = FALSE;

	for (DWORD i = 0; i < dwNumberOfObjects; i++)
	{
		if (dwObjectToFind == ((PERF_OBJECT_TYPE*)pObject)->ObjectNameTitleIndex)
		{
			fFoundObject = TRUE;
			break;
		}

		pObject += ((PERF_OBJECT_TYPE*)pObject)->TotalByteLength;
	}

	return (fFoundObject) ? (PERF_OBJECT_TYPE*)pObject : NULL;
}

// Use the counter index to find the object in the performance data.
PERF_COUNTER_DEFINITION* GetCounter(PERF_OBJECT_TYPE* pObject, DWORD dwCounterToFind)
{
	PERF_COUNTER_DEFINITION* pCounter = (PERF_COUNTER_DEFINITION*)((LPBYTE)pObject + pObject->HeaderLength);
	DWORD dwNumberOfCounters = pObject->NumCounters;
	BOOL fFoundCounter = FALSE;

	for (DWORD i = 0; i < dwNumberOfCounters; i++)
	{
		if (pCounter->CounterNameTitleIndex == dwCounterToFind)
		{
			fFoundCounter = TRUE;
			break;
		}

		pCounter++;
	}

	return (fFoundCounter) ? pCounter : NULL;
}


// Returns a pointer to the beginning of the PERF_COUNTER_BLOCK. The location of the 
// of the counter data block depends on whether the object contains single instance
// counters or multiple instance counters (see PERF_OBJECT_TYPE.NumInstances).
PERF_COUNTER_BLOCK* GetCounterBlock(PERF_OBJECT_TYPE* pObject, LPWSTR pInstanceName)
{
	PERF_COUNTER_BLOCK* pBlock = NULL;
	PERF_INSTANCE_DEFINITION* pInstance = NULL;

	// If there are no instances, the block follows the object and counter structures.
	pBlock = (PERF_COUNTER_BLOCK*)((LPBYTE)pObject + pObject->DefinitionLength);

	return pBlock;
}

// Retrieve the raw counter value and any supporting data needed to calculate
// a displayable counter value. Use the counter type to determine the information
// needed to calculate the value.
BOOL GetValue(PERF_OBJECT_TYPE* pObject,
	PERF_COUNTER_DEFINITION* pCounter,
	PERF_COUNTER_BLOCK* pCounterDataBlock,
	PRAW_DATA pRawData)
{
	PVOID pData = NULL;

	//Point to the raw counter data.
	pData = (PVOID)((LPBYTE)pCounterDataBlock + pCounter->CounterOffset);

	// Our object type is PERF_COUNTER_LARGE_RAWCOUNT which is just a large integer
	if (pCounter->CounterType == PERF_COUNTER_LARGE_RAWCOUNT) {
		pRawData->Data = *(ULONGLONG*)pData;
		return TRUE;
	}
	else {
		pRawData->Data = 0;
		return FALSE;
	}
}


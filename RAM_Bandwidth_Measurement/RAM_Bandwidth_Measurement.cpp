// Measures the RAM read and write bandwidth
//

#include "stdafx.h"
#include "windows.h"
#include "RAM_Bandwidth_Measurement.h"

static const INT REPS = 100; // The reps averaged
static const FLOAT MB = 0.2f; // Mega Bytes

// Big array for reading and writing from RAM
static const INT32 DATA_SIZE = 2000000;
int bigArray[DATA_SIZE];

// How we read 2 values
#define RAM_READ_2 \
	id1 = rand() % DATA_SIZE; \
	id2 = (id1 + DATA_SIZE/2)%DATA_SIZE; \
	sum = bigArray[id1] + bigArray[id2]; \
	DummyFunction(sum);

// Macro to read 10 values
#define RAM_READ_10 \
	RAM_READ_2\
	RAM_READ_2\
	RAM_READ_2\
	RAM_READ_2\
	RAM_READ_2

// Macro to read 100 values
#define RAM_READ_100 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 \
	RAM_READ_10 

// Macro to read 1000 values
#define RAM_READ_1000 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 \
	RAM_READ_100 

// Macro to read 10000 values
#define RAM_READ_10000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 \
	RAM_READ_1000 

// How we measure read overhead of 2 values
#define RAM_READ_OVERHEAD_2 \
	id1 = rand() % DATA_SIZE; \
	id2 = (id1 + DATA_SIZE/2)%DATA_SIZE; \
	sum = id1 + id2; \
	DummyFunction(sum);

// Macro to measure read overhead of 10 values
#define RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_2\
	RAM_READ_OVERHEAD_2\
	RAM_READ_OVERHEAD_2\
	RAM_READ_OVERHEAD_2\
	RAM_READ_OVERHEAD_2

// Macro to measure read overhead of 100 values
#define RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 \
	RAM_READ_OVERHEAD_10 

// Macro to measure read overhead of 1000 values
#define RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 \
	RAM_READ_OVERHEAD_100 

// Macro to measure read overhead of 10000 values
#define RAM_READ_OVERHEAD_10000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000 \
	RAM_READ_OVERHEAD_1000

// How we write 2 values
#define RAM_WRITE_2 \
	id1 = rand() % DATA_SIZE; \
	id2 = (id1 + DATA_SIZE/2)%DATA_SIZE; \
	bigArray[id1] = id2; \
	bigArray[id2] = id1; \

// Macro to write 10 values
#define RAM_WRITE_10 \
	RAM_WRITE_2\
	RAM_WRITE_2\
	RAM_WRITE_2\
	RAM_WRITE_2\
	RAM_WRITE_2

// Macro to write 100 values
#define RAM_WRITE_100 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 \
	RAM_WRITE_10 

// Macro to write 1000 values
#define RAM_WRITE_1000 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 \
	RAM_WRITE_100 

// Macro to write 10000 values
#define RAM_WRITE_10000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 \
	RAM_WRITE_1000 

// How we measure write overhead of 2 values
#define RAM_WRITE_OVERHEAD_2 \
	id1 = rand() % DATA_SIZE; \
	id2 = (id1 + DATA_SIZE/2)%DATA_SIZE;

// Macro to measure write overhead of 10 values
#define RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_2\
	RAM_WRITE_OVERHEAD_2\
	RAM_WRITE_OVERHEAD_2\
	RAM_WRITE_OVERHEAD_2\
	RAM_WRITE_OVERHEAD_2

// Macro to measure write overhead of 100 values
#define RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 \
	RAM_WRITE_OVERHEAD_10 

// Macro to measure write overhead of 1000 values
#define RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 \
	RAM_WRITE_OVERHEAD_100 

// Macro to measure write overhead of 10000 values
#define RAM_WRITE_OVERHEAD_10000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000 \
	RAM_WRITE_OVERHEAD_1000

// ---------------------------------------------------------------------------
// Dummy function
// ---------------------------------------------------------------------------
void DummyFunction(int value)
{
	return;
}

// ---------------------------------------------------------------------------
// memRAMBandwidthReadTest
// ---------------------------------------------------------------------------
LONGLONG memRAMBandwidthReadTest()
{
	LARGE_INTEGER startTimeStamp;
	LARGE_INTEGER endTimeStamp;
	LARGE_INTEGER frequency;

	int id1 = 0;
	int id2 = 0;
	int sum = 0;

	// Get the start time
	QueryPerformanceCounter(&startTimeStamp);

	RAM_READ_10000
		RAM_READ_10000
		RAM_READ_10000
		RAM_READ_10000
		RAM_READ_10000

		// Get the end time
		QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);

	return (endTimeStamp.QuadPart - startTimeStamp.QuadPart) * 1000000 / frequency.QuadPart;
}

// ---------------------------------------------------------------------------
// memRAMBandwidthReadOverheadTest
// ---------------------------------------------------------------------------
LONGLONG memRAMBandwidthReadOverheadTest()
{
	LARGE_INTEGER startTimeStamp;
	LARGE_INTEGER endTimeStamp;
	LARGE_INTEGER frequency;

	int id1 = 0;
	int id2 = 0;
	int sum = 0;

	// Get the start time
	QueryPerformanceCounter(&startTimeStamp);

	RAM_READ_OVERHEAD_10000
		RAM_READ_OVERHEAD_10000
		RAM_READ_OVERHEAD_10000
		RAM_READ_OVERHEAD_10000
		RAM_READ_OVERHEAD_10000

		// Get the end time
		QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);

	return (endTimeStamp.QuadPart - startTimeStamp.QuadPart) * 1000000 / frequency.QuadPart;
}

// ---------------------------------------------------------------------------
// memRAMBandwidthWriteTest
// ---------------------------------------------------------------------------
LONGLONG memRAMBandwidthWriteTest()
{
	LARGE_INTEGER startTimeStamp;
	LARGE_INTEGER endTimeStamp;
	LARGE_INTEGER frequency;

	int id1 = 0;
	int id2 = 0;

	// Get the start time
	QueryPerformanceCounter(&startTimeStamp);

	RAM_WRITE_10000
		RAM_WRITE_10000
		RAM_WRITE_10000
		RAM_WRITE_10000
		RAM_WRITE_10000

		// Get the end time
		QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);

	return (endTimeStamp.QuadPart - startTimeStamp.QuadPart) * 1000000 / frequency.QuadPart;
}

// ---------------------------------------------------------------------------
// memRAMBandwidthWriteOverheadTest
// ---------------------------------------------------------------------------
LONGLONG memRAMBandwidthWriteOverheadTest()
{
	LARGE_INTEGER startTimeStamp;
	LARGE_INTEGER endTimeStamp;
	LARGE_INTEGER frequency;

	int id1 = 0;
	int id2 = 0;

	// Get the start time
	QueryPerformanceCounter(&startTimeStamp);

	RAM_WRITE_OVERHEAD_10000
		RAM_WRITE_OVERHEAD_10000
		RAM_WRITE_OVERHEAD_10000
		RAM_WRITE_OVERHEAD_10000
		RAM_WRITE_OVERHEAD_10000

		// Get the end time
		QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);

	return (endTimeStamp.QuadPart - startTimeStamp.QuadPart) * 1000000 / frequency.QuadPart;
}

// ---------------------------------------------------------------------------
// GetRAM_ReadBandwidth
// ---------------------------------------------------------------------------
FLOAT GetRAM_ReadBandwidth()
{
	LONGLONG avg = 0;
	LONGLONG avgOverhead = 0;

	// init the array
	for (int i = 0; i<DATA_SIZE; i++)
	{
		bigArray[i] = i;
	}

	// average over the reps
	for (INT i = 0; i<REPS; i++)
	{
		avg += memRAMBandwidthReadTest() / REPS;
		avgOverhead += memRAMBandwidthReadOverheadTest() / REPS;
	}

	LONGLONG corrected = (avg - avgOverhead);
	return 200000.0f / corrected;
}

// ---------------------------------------------------------------------------
// GetRAM_WriteBandwidth
// ---------------------------------------------------------------------------
FLOAT GetRAM_WriteBandwidth()
{
	LONGLONG avg = 0;
	LONGLONG avgOverhead = 0;

	// init the array
	for (int i = 0; i<DATA_SIZE; i++)
	{
		bigArray[i] = i;
	}

	// average over the reps
	for (INT i = 0; i<REPS; i++)
	{
		avg += memRAMBandwidthWriteTest() / REPS;
		avgOverhead += memRAMBandwidthWriteOverheadTest() / REPS;
	}

	LONGLONG corrected = (avg - avgOverhead);
	return 200000.0f / corrected;
}

// Application entry point
int _tmain(int argc, _TCHAR* argv[])
{
	printf("RAM read bandwith in MB/s: %I64d \n", GetRAM_ReadBandwidth());
	printf("RAM write bandwith in MB/s: %I64d \n", GetRAM_WriteBandwidth());
	return 0;
}
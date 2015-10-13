// CPU_Measurement_Overhead.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

#define NUM_TESTS		10000			// take average time

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

double loop_overhead() {
	__int64 iTotalTime = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		for (int j = 0; j <= 0; j++) {}
		QueryPerformanceCounter(&end_time_stamp);
		iTotalTime += time_elapsed();
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return dTotalTime / NUM_TESTS;
}

void write_results(char * fileName, double dMeasurementOverhead, double dLoopOverhead) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, fileName, "w");
	if (err) {
		printf("The file %s was not opened\n", fileName);
		return;
	}
	fprintf(fp, "Overhead \t\t\t Time (ns)\n");  //print results to file
	fprintf(fp, "%-*s\t\t%*f\n", 16, "Measurement", 10, dMeasurementOverhead);
	fprintf(fp, "%-*s\t\t%*f\n", 16, "Loop", 10, dLoopOverhead);
	fclose(fp);
}

int _tmain(int argc, _TCHAR* argv[])
{
	double dMeasurementOverhead, dLoopOverhead;
	dMeasurementOverhead = measurement_overhead();
	dLoopOverhead = loop_overhead() - dMeasurementOverhead;
	write_results("CPU_Measurement_Overhead_Results.txt", dMeasurementOverhead, dLoopOverhead);
	return 0;
}


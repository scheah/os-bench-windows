// CPU_Procedure_Overhead.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

#define NUM_TESTS		10000			// take average time for each procedure
#define NUM_ARGS		7

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

void procedure0() {}
void procedure1(int a1) {}
void procedure2(int a1, int a2) {}
void procedure3(int a1, int a2, int a3) {}
void procedure4(int a1, int a2, int a3, int a4) {}
void procedure5(int a1, int a2, int a3, int a4, int a5) {}
void procedure6(int a1, int a2, int a3, int a4, int a5, int a6) {}
void procedure7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {}

double procedureTime(int numArgs) {
	__int64 iTotalTime = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		switch (numArgs) {
		case 0:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure0();
			QueryPerformanceCounter(&end_time_stamp);
			break;
		case 1:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure1(1);
			QueryPerformanceCounter(&end_time_stamp);
			break;
		case 2:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure2(1, 2);
			QueryPerformanceCounter(&end_time_stamp);
			break;
		case 3:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure3(3, 2, 1);
			QueryPerformanceCounter(&end_time_stamp);
			break;
		case 4:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure4(1, 2, 3, 4);
			QueryPerformanceCounter(&end_time_stamp);
			break;
		case 5:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure5(5, 4, 3, 2, 1);
			QueryPerformanceCounter(&end_time_stamp);
			break;
		case 6:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure6(1, 2, 3, 4, 5, 6);
			QueryPerformanceCounter(&end_time_stamp);
			break;
		case 7:
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			procedure7(7, 6, 5, 4, 3, 2, 1);
			QueryPerformanceCounter(&end_time_stamp);
			break;
		default:
			return 0; //not valid
			break;
		}
		iTotalTime += time_elapsed();
	}
	double dTotalTime = static_cast<double>(iTotalTime);
	return (dTotalTime / NUM_TESTS);
}

void write_results(char * fileName, double results[], int size) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, fileName, "w");
	if (err) {
		printf("The file %s was not opened\n", fileName);
		return;
	}
	fprintf(fp, "# Arguments \t\t\t Time (ns)\n");  //print results to file
	for (int i = 0; i < size; i++) {
		fprintf(fp, "%*d\t\t%*f\n", 16, i, 10, results[i]);
	}
	fclose(fp);
}

int _tmain(int argc, _TCHAR* argv[])
{
	const int size = NUM_ARGS+1;
	double results[size];
	double dMeasurementOverhead = measurement_overhead();
	for (int i = size-1; i >= 0; i--) {
		results[i] = procedureTime(i) - dMeasurementOverhead;
	}
	write_results("CPU_Procedure_Overhead_Results.txt", results, size);
	return 0;
}


// Network_Connection_Overhead.cpp : Defines the entry point for the console application.
//

//Plan: measure setup and teardown of just tcp

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// set remote target ip here, should set to some other IP assigned by your router 
#define REMOTE_IP_ADDR		"192.168.2.2"	// change to remote ip with TCP echo server running
#define LOOPBACK_IP_ADDR	"127.0.0.1" // do not change

#define NUM_CONNECTS		50			// take average of all the sets of connect+teardowns

#define TCP_PORT			27777			// specify tcp port here

// For timing
static LARGE_INTEGER freq;
static LARGE_INTEGER start_time_stamp;
static LARGE_INTEGER end_time_stamp;

__int64 time_elapsed() { //in ms
	LARGE_INTEGER elapsed_time;
	elapsed_time.QuadPart = (end_time_stamp.QuadPart - start_time_stamp.QuadPart);
	elapsed_time.QuadPart *= (LONGLONG)1000.0;
	elapsed_time.QuadPart /= freq.QuadPart;
	return elapsed_time.QuadPart;
}

double tcp_connect_overhead(bool bLoopback = 0) { //remote tcp by default
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	// Uses the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h 
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
#ifdef _DEBUG
		printf("WSAStartup failed with error: %d\n", err);
#endif
		return 1;
	}
	LPCTSTR targetAddr;
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(TCP_PORT);
	if (bLoopback)
		targetAddr = TEXT(LOOPBACK_IP_ADDR);
	else
		targetAddr = TEXT(REMOTE_IP_ADDR);
	int success = InetPton(AF_INET, targetAddr, &sockaddr.sin_addr);
	if (!success) {
#ifdef _DEBUG
		printf("Error with InetPton\n");
#endif
		return -1;
	}
	__int64 iTotalTimeElapsed = 0;
	for (int i = 0; i < NUM_CONNECTS; i++) {
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) {
#ifdef _DEBUG
			printf("Error with socket\n");
#endif
			return -1;
		}
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start_time_stamp);
		if (connect(sock, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
#ifdef _DEBUG
			printf("Error with connect\n");
			printf("connect returned error: %ld\n", WSAGetLastError());
#endif
			return -1;
		}
		else { //successfully connected 
#ifdef _DEBUG
			printf("Success %d\n", i);
#endif
		}
		closesocket(sock);
		QueryPerformanceCounter(&end_time_stamp);
		iTotalTimeElapsed += time_elapsed();
	}
	WSACleanup();

	printf("total time: %I64u ms\n", iTotalTimeElapsed);
	double dtotalTime = static_cast<double>(iTotalTimeElapsed);
	return dtotalTime / NUM_CONNECTS;
}

void write_results(char * fileName, double dTcpConnectOverheadRemote, double dTcpConnectOverheadLoopback) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, fileName, "w");
	if (err) {
		printf("The file %s was not opened\n", fileName);
		return;
	}
	fprintf(fp, "Type \t\t\t Connect Overhead (ms)\n");  //print results to file
	fprintf(fp, "%*s\t\t%*f\n", 16, "TCP Remote", 10, dTcpConnectOverheadRemote);
	fprintf(fp, "%*s\t\t%*f\n", 16, "TCP Loopback", 10, dTcpConnectOverheadLoopback);
	fclose(fp);
}

int _tmain(int argc, _TCHAR* argv[])
{
	double tcpConnectOverheadRemote = tcp_connect_overhead();
	double tcpConnectOverheadLoopback = tcp_connect_overhead(1);
	write_results("Network_Connection_Overhead_Results.txt", tcpConnectOverheadRemote, tcpConnectOverheadLoopback);
	return 0;
}
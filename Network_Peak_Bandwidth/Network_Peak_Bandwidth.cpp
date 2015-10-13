// Network_Peak_Bandwidth.cpp : Defines the entry point for the console application.
//
/*Plan: a PEAK bandwidth would be, if transferring a fixed amount of data, would be the data
transfer with the highest throughput (shortest time)

Note the server you're connecting to should not be sending data back to the client.
We will measure only the throughput of an outgoing message from the client.
*/

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// set remote target ip here, should set to some other IP assigned by your router 
#define REMOTE_IP_ADDR		"192.168.2.4"	// change to remote ip with TCP echo server running
#define LOOPBACK_IP_ADDR	"127.0.0.1" // do not change

#define NUM_PACKETS			30			// take average RTT of NUM_PACKETS packets

#define TCP_PORT			27777			// specify tcp port here

#define MSG_SIZE			4096 		// in bytes

// For timing
static LARGE_INTEGER freq;
static LARGE_INTEGER start_time_stamp;
static LARGE_INTEGER end_time_stamp;

__int64 time_elapsed() { //in microseconds
	LARGE_INTEGER elapsed_time;
	elapsed_time.QuadPart = (end_time_stamp.QuadPart - start_time_stamp.QuadPart);
	elapsed_time.QuadPart *= (LONGLONG)1000000.0;
	elapsed_time.QuadPart /= freq.QuadPart;
	return elapsed_time.QuadPart;
}

double tcp_min_time(bool bLoopback = 0) { //remote tcp by default
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
	long totalRTT = 0;
	LPCTSTR targetAddr;
	struct sockaddr_in sockaddr;

	//u_long iMode = 0;
	//ioctlsocket(sock, FIONBIO, &iMode); //blocking
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
	// data to send. add extra character for null character
	char * szSendMessage = new char[MSG_SIZE + 1];
	char * szBuffer = new char[MSG_SIZE + 1];
	for (int i = 0; i < MSG_SIZE; i++) szSendMessage[i] = '1';
	szSendMessage[MSG_SIZE] = '\0';
	int iBytesToRecv = strlen(szSendMessage); // echo server returns what we send
	__int64 iLowestTime = MAXINT64;
	
	for (int i = 0; i < NUM_PACKETS; i++)  {
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) {
#ifdef _DEBUG
			printf("Error with socket\n");
#endif
			return -1;
		}
		int size = 0;
		int nodelay = 1;
		if (!bLoopback) {
			setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
		}
		//setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&size, sizeof(size));
		if (connect(sock, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
#ifdef _DEBUG
			printf("Error with connect\n");
			printf("connect returned error: %ld\n", WSAGetLastError());
#endif
			return -1;
		}
		else { //successfully connected
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&start_time_stamp);
			int iSent = send(sock, szSendMessage, strlen(szSendMessage), MSG_OOB);
			QueryPerformanceCounter(&end_time_stamp);
#ifdef _DEBUG
			printf("Sent tcp message to %s\n", bLoopback ? LOOPBACK_IP_ADDR : REMOTE_IP_ADDR);
			printf("\tSent %d bytes\n", iSent);
			printf("\t\tElapsed time: %I64u us\n", time_elapsed());
#endif
			__int64 time = time_elapsed();
			if (time < iLowestTime)
				iLowestTime = time;

			closesocket(sock);
			//free(szSendMessage);
		}
	}
	
	free(szSendMessage);
	free(szBuffer);
	
	WSACleanup();
	printf("Shortest time: %I64u microseconds\n", iLowestTime);
	double dLowestTime = static_cast<double>(iLowestTime);
	return dLowestTime;
}

double tcp_peak_bandwidth(bool bLoopback = 0) {// bytes/s
	double dMinTime = tcp_min_time(bLoopback);
	double dPeakBandWidth;
	dPeakBandWidth = (double)(MSG_SIZE) / dMinTime * 8.0 * 1000000 / 1048576; // Mb/s
	return dPeakBandWidth;
}

void write_results(char * fileName, double dTcpPeakBandwidthRemote, double dTcpPeakBandwidthLoopback) {
	FILE *fp;
	errno_t err;

	err = fopen_s(&fp, fileName, "w");
	if (err) {
		printf("The file %s was not opened\n", fileName);
		return;
	}
	fprintf(fp, "Type \t\t\t Peak Bandwidth (Mb/s)\n");  //print results to file
	fprintf(fp, "%*s\t\t%*f\n", 16, "TCP Remote", 10, dTcpPeakBandwidthRemote);
	fprintf(fp, "%*s\t\t%*f\n", 16, "TCP Loopback", 10, dTcpPeakBandwidthLoopback);
	fclose(fp);
}

int _tmain(int argc, _TCHAR* argv[])
{
	double tcpPeakBandwidthRemote = tcp_peak_bandwidth(0);
	double tcpPeakBandwidthLoopback = tcp_peak_bandwidth(1);
	write_results("Network_Peak_Bandwidth_Results.txt", tcpPeakBandwidthRemote, tcpPeakBandwidthLoopback);
	return 0;
}


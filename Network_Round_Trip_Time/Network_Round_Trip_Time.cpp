// Network_Round_Trip_Time.cpp : Defines the entry point for the console application.
//
 
#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <stdio.h>
 
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
 
// set remote target ip here, should set to some other IP assigned by your router 
#define REMOTE_IP_ADDR      "192.168.2.13"  // change to remote ip with TCP echo server running
#define LOOPBACK_IP_ADDR    "127.0.0.1" // do not change
 
#define NUM_PACKETS         1000           // take average RTT of NUM_PACKETS packets
 
#define TCP_PORT            27777           // specify tcp port here
 
#define MSG_SIZE            4096            // in bytes
 
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
 
double tcp_rtt(bool bLoopback = 0) { //remote tcp by default
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
 
    int iTimeOut = 5000;
    long totalRTT = 0;
    LPCTSTR targetAddr;
    struct sockaddr_in sockaddr;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
#ifdef _DEBUG
        printf("Error with socket\n");
#endif
        return -1;
    }
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
    char * szSendMessage = new char[MSG_SIZE+1];
    char * szBuffer = new char[MSG_SIZE+1];
    for (int i = 0; i < MSG_SIZE; i++) szSendMessage[i] = '1';
    szSendMessage[MSG_SIZE] = '\0';
    int iBytesToRecv = strlen(szSendMessage); // echo server returns what we send
    __int64 iTotalTime = 0;
    if (connect(sock, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
#ifdef _DEBUG
        printf("Error with connect\n");
        printf("connect returned error: %ld\n", WSAGetLastError());
#endif
        return -1;
    }
    else { //successfully connected
        for (int i = 0; i < NUM_PACKETS; i++)  {// depends on whether or not we have to reconnect
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&start_time_stamp);
            int iSent = send(sock, szSendMessage, strlen(szSendMessage), 0);
            int iRecv = recv(sock, szBuffer, iBytesToRecv, 0);
            QueryPerformanceCounter(&end_time_stamp);
#ifdef _DEBUG
            printf("Sent tcp message to %s\n", bLoopback ? LOOPBACK_IP_ADDR : REMOTE_IP_ADDR);
            printf("\tSent %d bytes\n", iSent);
            printf("\tReceived %d bytes\n", iRecv);
            printf("\t\tElapsed time: %I64u ms\n", time_elapsed());
#endif
            iTotalTime += time_elapsed();
        }
    }
    free(szSendMessage);
    free(szBuffer);
    closesocket(sock);
    WSACleanup();
    double dTotalTime = static_cast<double>(iTotalTime);
    return dTotalTime/NUM_PACKETS;
 
}
 
double icmp_rtt(bool bLoopback = 0) { // remote icmp by default
    __int64 totalRTT = 0;
    HANDLE hIcmpFile = IcmpCreateFile();
    INT ipaddr; // binary representation
    LPCTSTR targetAddr; 
    if (bLoopback)
        targetAddr = TEXT(LOOPBACK_IP_ADDR);
    else
        targetAddr = TEXT(REMOTE_IP_ADDR);
    int success = InetPton(AF_INET, targetAddr, &ipaddr);
    if (!success) {
#ifdef _DEBUG
        printf("Error with InetPton\n");
#endif
        return -1;
    }
    DWORD dwRetVal = 0;
    char * szSendMessage = new char[MSG_SIZE + 1];
    for (int i = 0; i < MSG_SIZE; i++) szSendMessage[i] = '1';
    szSendMessage[MSG_SIZE] = '\0';
 
    DWORD dwReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(szSendMessage);
    LPVOID lpReplyBuffer = (VOID*)malloc(dwReplySize);
    PICMP_ECHO_REPLY pEchoReply;
    for (int i = 0; i < NUM_PACKETS; i++) {
        dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, szSendMessage, sizeof(szSendMessage), NULL, lpReplyBuffer, dwReplySize, 5000); // 5s timeout
        if (dwRetVal == 0) { //error
#ifdef _DEBUG
            printf("Call to IcmpSendEcho failed.\n");
            printf("IcmpSendEcho returned error: %ld\n", GetLastError());
#endif
            return -1;
        }
        else {
            pEchoReply = (PICMP_ECHO_REPLY)lpReplyBuffer;
#ifdef _DEBUG
            printf("Sent icmp message to %s\n", bLoopback ? LOOPBACK_IP_ADDR : REMOTE_IP_ADDR);
            if (dwRetVal > 1) { //more than one reply 
                printf("\tReceived %ld icmp message responses\n", dwRetVal);
                printf("\tInformation from the first response:\n");
            }
            else {
                printf("\tReceived %ld icmp message response\n", dwRetVal);
                printf("\tInformation from this response:\n");
            }
            printf("\t  Status = %ld\n", pEchoReply->Status);
            printf("\t  Roundtrip time = %ld ms\n", pEchoReply->RoundTripTime);
#endif
            totalRTT += pEchoReply->RoundTripTime;
        }
    }
    double dTotalTime = static_cast<double>(totalRTT);
    return dTotalTime / NUM_PACKETS;
}
 
void write_results(char * fileName, double iICMPRemoteRTT, double iICMPLoopbackRTT, double iTCPremoteRTT, double iTCPloopbackRTT) {
    FILE *fp;
    errno_t err;
 
    err = fopen_s(&fp, fileName, "w");
    if (err) {
        printf("The file %s was not opened\n", fileName);
        return;
    }
    fprintf(fp, "Type \t\t\t Round trip time (ms)\n");  //print results to file
    fprintf(fp, "%*s\t\t%*f\n", 16, "ICMP Remote",      10, iICMPRemoteRTT);
    fprintf(fp, "%*s\t\t%*f\n", 16, "ICMP Loopback",    10, iICMPLoopbackRTT);
    fprintf(fp, "%*s\t\t%*f\n", 16, "TCP Remote",       10, iTCPremoteRTT);
    fprintf(fp, "%*s\t\t%*f\n", 16, "TCP Loopback",     10, iTCPloopbackRTT);
    fclose(fp);
}
 
int _tmain(int argc, _TCHAR* argv[])
{
    double iICMPremoteRTT = icmp_rtt();
    double iICMPloopbackRTT = icmp_rtt(1);  //loopback option on
    double iTCPremoteRTT = tcp_rtt();
    double iTCPloopbackRTT = tcp_rtt(1);    //loopback option on
    write_results("Network_Round_Trip_Time_Results.txt", iICMPremoteRTT, iICMPloopbackRTT, iTCPremoteRTT, iTCPloopbackRTT);
    return 0;
}
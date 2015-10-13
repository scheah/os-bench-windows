// Network_TCP_Echo_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "ws2_32.lib")

#define TCP_PORT			"27777"			// specify tcp port here

int MSG_SIZE = 0;
bool ECHOBACK = 0;


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3) {
		printf("Usage: Network_TCP_Echo_Server.exe MSG_SIZE ECHOBACK\n");
		return 0;
	}
	else {
		MSG_SIZE = _ttoi(argv[1]);
		ECHOBACK = _ttoi(argv[2]);
		printf("MSG_SIZE set to %d bytes\nECHOBACK set to %d\n", MSG_SIZE, ECHOBACK);
	}
	while (true) {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			printf("WSAStartup failed with error: %d\n", err);
			return 1;
		}

		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;
		struct addrinfo *result = NULL;
		struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;
		// Resolve the server address and port
		int iResult = getaddrinfo(NULL, TCP_PORT, &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return 1; 
		}
		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1; 
		}
		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		freeaddrinfo(result);
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		// No longer need server socket
		closesocket(ListenSocket);
		// Receive until the peer shuts down the connection
		int iSendResult;
		char * recvbuf = new char[MSG_SIZE + 1];
		int recvbuflen = MSG_SIZE;
		do {

			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				// Echo the buffer back to the sender
				if (ECHOBACK)
					iSendResult = send(ClientSocket, recvbuf, iResult, 0);
#ifdef _DEBUG
				printf("Bytes received: %d\n", iResult);
				if (ECHOBACK && iSendResult == SOCKET_ERROR) {
					printf("minor: send failed with error: %d\n", WSAGetLastError());
				}
				if (ECHOBACK)
					printf("Bytes sent: %d\n", iSendResult);
#endif
			}
#ifdef _DEBUG
			else if (iResult == 0)
				printf("Connection closing...\n");
			else  {
				printf("minor: recv failed with error: %d\n", WSAGetLastError());
			}
#endif
		} while (iResult > 0);

		// shutdown the connection since we're done
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
		}
		// cleanup
		closesocket(ClientSocket);
		WSACleanup();
	}
	return 0;
}
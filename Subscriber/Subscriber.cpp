#include "stdafx.h"

void InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
}

void connectToServer(SOCKET* connectSocket, char* ipv4_address, u_int port)
{
	*connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*connectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ipv4_address);
	serverAddress.sin_port = htons(port);

	if (connect(*connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
		printf("Unable to connect to server. Error code: %d\n", WSAGetLastError());
		closesocket(*connectSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
}

void disconnect(SOCKET* socket) {
	closesocket(*socket);
	WSACleanup();
	printf("Connection closed.\n");
}


void input_message(char* message) {
	printf("Input message (one character) :\n");
	scanf(" %c", message);
}

void input_topic(char* topic) {
	printf("Input topic (one character) :\n");
	scanf(" %c", topic);
}

void subscribe(SOCKET * socket, char topic)
{
	int iResult = send(*socket,&topic, 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Error occured while subscribing...\n");
	}
	else {
		printf("Woow NICE!Now you are subscribed for %c topic!\n",topic);
	}
}

void make_data_package(char message, char topic, char* data_package) {
	data_package[0] = topic;
	data_package[1] = message;
}

void waitForMessage(SOCKET * socket,unsigned buffer_size)
{


	int iResult;
	SOCKET acceptedSocket;
	SOCKET listenSocket = *socket;

	char *recvbuf = (char*)malloc(buffer_size);

	printf("\n Waiting for messages...\n");
	do
	{
		// Wait for clients and accept client connections.
		// Returning value is acceptedSocket used for further
		// Client<->Server communication. This version of
		// server will handle only one client.
		acceptedSocket = accept(listenSocket, NULL, NULL);

		if (acceptedSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		do
		{
			// Receive data until the client shuts down the connection
			iResult = recv(acceptedSocket, recvbuf, buffer_size, 0);
			if (iResult > 0)
			{
				printf("Message received from client: %s.\n", recvbuf);
			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				printf("Connection with client closed.\n");
				closesocket(acceptedSocket);
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
			}
		} while (iResult > 0);

		// here is where server shutdown loguc could be placed

	} while (1);
}

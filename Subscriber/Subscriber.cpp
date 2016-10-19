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
	int iResult = send(*socket, &topic, 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Error occured while subscribing...\n");
	}
	else {
		printf("Woow NICE!Now you are subscribed for %c topic!\n", topic);
	}
}

void make_data_package(char message, char topic, char* data_package) {
	data_package[0] = topic;
	data_package[1] = message;
}

void waitForMessage(SOCKET * socket, unsigned buffer_size)
{
	int iResult;
	char *recvbuf = (char*)malloc(buffer_size);
	//set parameter for NonBlocking mode
	set_nonblocking_mode(socket);

	printf("\n Waiting for messages...\n");
	do
	{
		iResult = Select(socket);

		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}
		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(SERVER_SLEEP_TIME);
			continue;
		}

		// Receive data until the client shuts down the connection
		iResult = recv(*socket, recvbuf, buffer_size, 0);
		if (iResult > 0)
		{
			recvbuf[iResult] = NULL;
			printf("Message received from client: %s.\n", recvbuf);
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(*socket);
			exit(EXIT_FAILURE);
		}
		else
		{
			// there was an error during recv
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(*socket);
			exit(EXIT_FAILURE);
		}
		// here is where server shutdown loguc could be placed

	} while (1);
}

void checkConfimation(SOCKET *socket) {

	int iResult;
	char recvbuf;

	bool successSubscribed = false; 

	set_nonblocking_mode(socket);
	while (!successSubscribed) {

		iResult = Select(socket);

		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}
		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(SERVER_SLEEP_TIME);
			continue;
		}

		// Receive data until the client shuts down the connection
		iResult = recv(*socket, &recvbuf, 1, 0);

		if (iResult > 0)
		{
			if (recvbuf == SUBSCRIBE_SUCCESS) {
				printf("Successfuly subscribed!\n");
				successSubscribed = true;
			}
			else {
				printf("Error while subscribing!\n");
				closesocket(*socket);
				getchar();
				exit(EXIT_FAILURE);
			}
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(*socket);
			exit(EXIT_FAILURE);
		}
		else
		{
			// there was an error during recv
			printf("recv failed with error while subscribing: %d\n", WSAGetLastError());
			closesocket(*socket);
			exit(EXIT_FAILURE);
		}
	}
}

void set_nonblocking_mode(SOCKET * socket)
{
	// Set socket to nonblocking mode
	unsigned long int nonBlockingMode = 1;
	int iResult = ioctlsocket(*socket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		closesocket(*socket);
		exit(EXIT_FAILURE);
	}
}

int Select(SOCKET * socket)
{
	// Initialize select parameters
	FD_SET set;
	timeval timeVal;

	FD_ZERO(&set);
	// Add socket we will wait to read from
	FD_SET(*socket, &set);

	// Set timeouts to zero since we want select to return
	// instantaneously
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	return 	select(0 /* ignored */, &set, NULL, NULL, &timeVal);
}

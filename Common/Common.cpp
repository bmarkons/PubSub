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
void start_listening(SOCKET* listenSocket, char* port) {
	int iResult;

	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

									 // Resolve the server address and port
	iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Create a SOCKET for connecting to server
	*listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (*listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(*listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	// Set listenSocket in listening mode
	iResult = listen(*listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

}
bool send_nonblocking(SOCKET* socket, ByteArray package) {
	// send package in non blocking mode
	set_nonblocking_mode(socket);
	bool success = send_all(socket, package);
	// free sent package data
	free(package.array);

	if (!success) {
		closesocket(*socket);
	}

	return success;
}
bool send_all(SOCKET* socket, ByteArray package) {
	int iResult;
	int total_sent = 0;
	int byte_to_send;
	do {
		// Check if socket is ready for send
		int ready = is_ready_for_send(socket);

		if (ready == SOCKET_ERROR) {
			return false;
		}
		else if (ready == 0) {
			continue;
		}
		else {
			byte_to_send = package.size - total_sent;
			iResult = send(*socket, package.array + total_sent, byte_to_send, 0);
			if (iResult == SOCKET_ERROR) {
				return false;
			}
			total_sent += iResult;
		}
	} while (total_sent < package.size);

	return true;
}
void wait_for_message(SOCKET * socket, void* param, bool single_receive, messageHandler message_handler)
{
	char *recvbuf = NULL;
	set_nonblocking_mode(socket);

	printf("Waiting for messages...\n");
	do
	{
		int ready = is_ready_for_receive(socket);
		if (ready > 0) {
			if (receive(socket, &recvbuf)) {
				message_handler(socket, recvbuf, param);
				if (single_receive) {
					break;
				}
			}
			else {
				printf("Error occured while receiving message from socket.\n");
				closesocket(*socket);
				break;
			}
		}
		else if (ready < 0) {
			break;
		}
	} while (true);
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
		//exit(EXIT_FAILURE);
	}
}
int is_ready_for_receive(SOCKET * socket) {
	FD_SET set;
	timeval timeVal;

	FD_ZERO(&set);
	FD_SET(*socket, &set);

	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	int iResult = select(0, &set, NULL, NULL, &timeVal);

	if (iResult == SOCKET_ERROR)
	{
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
	}
	else if (iResult == 0) {
		Sleep(DEFAULT_SLEEP_TIME);
	}

	return iResult;
}
int is_ready_for_send(SOCKET * socket) {
	FD_SET set;
	timeval timeVal;

	FD_ZERO(&set);
	FD_SET(*socket, &set);

	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	int iResult = select(0, NULL, &set, NULL, &timeVal);

	if (iResult == SOCKET_ERROR) {
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
	}
	else if (iResult == 0) {
		Sleep(DEFAULT_SLEEP_TIME);
	}

	return iResult;
}
int recv_all(SOCKET* socket, char* recvbuff, int message_length) {

	int total_received = 0;

	int iResult;
	int byte_to_receive;
	do {
		byte_to_receive = message_length - total_received;
		if (byte_to_receive > DEFAULT_BUFLEN) {
			byte_to_receive = DEFAULT_BUFLEN;
		}
		iResult = recv(*socket, recvbuff + total_received, byte_to_receive, 0);
		if (iResult < 0) {
			printf("Unable to connect to server. Error code: %d\n", WSAGetLastError());
			return iResult;
		}
		total_received += iResult;
	} while (total_received < message_length);

	return iResult;
}
bool receive(SOCKET* socket, char** recvbuf) {

	int iResult;
	u_short message_length = 0;

	char header[MAIN_HEADER_SIZE];
	iResult = recv_all(socket, header, MAIN_HEADER_SIZE);
	memcpy(&message_length, header, sizeof(u_short));

	if (iResult < 0) {
		return false;
	}

	*recvbuf = (char*)calloc(message_length + 1, sizeof(char));
	iResult = recv_all(socket, *recvbuf, message_length);

	if (iResult < 0) {
		free(*recvbuf);
		return false;
	}

	return true;
}
void input_topic(char* topic) {
	printf("Input topic: ");
	scanf(" %s", topic);
}
void input_message(char* message) {
	printf("Input message (one character) :\n");
	scanf(" %s", message);
}

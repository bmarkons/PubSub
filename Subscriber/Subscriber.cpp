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

void input_topic(char* topic) {
	printf("Input topic (one character) : ");
	scanf(" %s", topic);
}

void subscribe(SOCKET * socket, ByteArray topic)
{
	ByteArray package = make_package(topic);

	bool success = send_nonblocking(socket, package);
	if (success) {
		printf("Subscribing on topic '%s'.\n", topic.array);
	}
	else {
		printf("Error occured while subscribing...\n");
	}
}

bool send_nonblocking(SOCKET* socket, ByteArray package) {
	set_nonblocking_mode(socket);
	while (true) {
		bool success;
		if (is_ready_for_send(socket)) {
			success = send_all(socket, package);
			free(package.array);
			if (!success) {
				closesocket(*socket);
			}
			return success;
		}
	}
}

bool send_all(SOCKET* socket, ByteArray package) {
	int iResult;
	int total_sent = 0;
	do {
		iResult = send(*socket, package.array + total_sent, package.size - total_sent, 0);
		total_sent += iResult;
	} while (total_sent < package.size);

	return iResult == SOCKET_ERROR ? false : true;
}

ByteArray make_package(ByteArray topic) {
	ByteArray package;
	package.size = topic.size + 4;
	package.array = (char*)calloc(package.size + 1, sizeof(char));

	u_short main_header = package.size - 2;
	u_short topic_header = topic.size;
	memcpy(package.array, &main_header, sizeof(u_short));
	memcpy(package.array + 2, &topic_header, sizeof(u_short));
	memcpy(package.array + 4, topic.array, topic.size);

	return package;
}

void wait_for_message(SOCKET * socket, unsigned buffer_size)
{
	int iResult;
	char *recvbuf = NULL;
	//set parameter for NonBlocking mode
	set_nonblocking_mode(socket);

	printf("Waiting for messages...\n");
	do
	{
		bool success;
		if (is_ready_for_receive(socket)) {
			success = receive(socket, &recvbuf);
			if (success) {
				printf("Message received from client: %s.\n", recvbuf + 2);
				printf("%d\n", strlen(recvbuf + 2));
			}
			else {
				printf("Error occured while receiving message from socket.\n");
				closesocket(*socket);
				break;
			}
		}
	} while (true);
}

void checkConfimation(SOCKET *socket) {
	int iResult;
	char *recvbuf = NULL;
	//set parameter for NonBlocking mode
	set_nonblocking_mode(socket);

	while (true)
	{
		bool success;
		if (is_ready_for_receive(socket)) {
			success = receive(socket, &recvbuf);
			if (success) {
				if (recvbuf[2] == SUBSCRIBE_SUCCESS) {
					printf("SUCCESS!\n");
				}
				else {
					printf("FAIL!\n");
					closesocket(*socket);
				}
				break;
			}
			else {
				printf("recv failed with error while subscribing: %d\n.\n", WSAGetLastError());
				closesocket(*socket);
				break;
			}
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
		//exit(EXIT_FAILURE);
	}
}

bool is_ready_for_receive(SOCKET * socket) {
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

	int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
	// lets check if there was an error during select
	if (iResult == SOCKET_ERROR)
	{
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
		return false;
	}
	// now, lets check if there are any sockets ready
	if (iResult == 0)
	{
		// there are no ready sockets, sleep for a while and check again
		Sleep(SERVER_SLEEP_TIME);
		return false;
	}

	return true;
}

bool is_ready_for_send(SOCKET * socket) {
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

	int iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal); //3rd parametar is set because need select for send
	// lets check if there was an error during select
	if (iResult == SOCKET_ERROR)
	{
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
		return false;
	}
	// now, lets check if there are any sockets ready
	if (iResult == 0)
	{
		// there are no ready sockets, sleep for a while and check again
		Sleep(SERVER_SLEEP_TIME);
		return false;
	}

	return true;
}

bool receive(SOCKET* socket, char** recvbuf) {
	u_int message_length = 0;
	int iResult;
	int total_received = 0;
	bool firstRecv = true;
	do {
		if (firstRecv) {
			char header[MAIN_HEADER_SIZE];
			iResult = recv(*socket, header, MAIN_HEADER_SIZE, 0);
			memcpy(&message_length, header, sizeof(u_short));
			*recvbuf = (char*)calloc(message_length + 1, sizeof(char));
			firstRecv = false;
		}
		else {
			iResult = recv(*socket, *recvbuf + total_received, DEFAULT_BUFLEN, 0);
			total_received += iResult;
		}
		if (iResult < 0) {
			break;
		}
	} while (total_received < message_length);

	(*recvbuf)[total_received] = NULL;  //set the end of the string
	return iResult < 0 ? false : true;
}

void subscribing(SOCKET* connectSocket) {
	char command;
	do {
		system("cls");
		char topic[MAX_INPUT_SIZE];
		input_topic(topic);

		//subsribing for specific topic
		ByteArray topic_bytes;
		topic_bytes.size = strlen(topic);
		topic_bytes.array = topic;
		subscribe(connectSocket, topic_bytes);

		//check if server returns a message that everything is ok
		checkConfimation(connectSocket);

		printf("Continue with subscribing on topic? [y/n]");
		scanf(" %c", &command);
	} while (command != 'n');
}

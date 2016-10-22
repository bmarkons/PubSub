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

void publishing_loop(SOCKET* socket) {
	char command;
	while (true) {
		printf("Do you want to publish message? [y/n]");
		scanf(" %c", &command);
		system("cls");

		if (command == 'n') {
			disconnect(socket);
			exit(EXIT_SUCCESS);
		}

		char message[MAX_INPUT_SIZE];
		char topic[MAX_INPUT_SIZE];
		input_message(message);
		input_topic(topic);
		system("cls");

		// make ByteArray structs
		ByteArray message_bytes;
		message_bytes.size = strlen(message);
		message_bytes.array = message;
		ByteArray topic_bytes;
		topic_bytes.size = strlen(topic);
		topic_bytes.array = topic;

		publish(message_bytes, topic_bytes, socket);
	}
}

void input_message(char* message) {
	printf("Input message (one character) :\n");
	scanf(" %s", message);
}

void input_topic(char* topic) {
	printf("Input topic (one character) :\n");
	scanf(" %s", topic);
}

void publish(ByteArray message, ByteArray topic, SOCKET* socket) {
	ByteArray package = make_package(message, topic);
	bool success = send_nonblocking(socket, package);
	if (success) {
		printf("Awesome! Message '%s' published on topic '%s'!\n", message, topic);
	}
	else {
		printf("Error occured while publishing...\n");
	}
}

ByteArray make_package(ByteArray message, ByteArray topic) {
	ByteArray package;

	package.size = PACKAGE_HEADER_SIZE + topic.size + message.size;
	package.array = (char*)malloc(package.size * sizeof(char));

	package.array[0] = package.size - 1;	// size of the rest of package
	package.array[1] = topic.size;			// size of topic
	package.array[2] = message.size;		// size of message

	memcpy(package.array + PACKAGE_HEADER_SIZE, topic.array, topic.size);
	memcpy(package.array + (PACKAGE_HEADER_SIZE + topic.size), message.array, message.size);

	return package;
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
#include "stdafx.h"

#pragma region SOCKETS

void InitializeWindowsSockets() {
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
}

void setNonBlockingMode(SOCKET * socket)
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

int ready_for_receive(SOCKET * socket) {
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

	return select(0 /* ignored */, &set, NULL, NULL, &timeVal);
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

#pragma endregion

#pragma region TOPIC_LIST

void free_topic_content(void * data) {
	free(*(TopicContent**)data);
}

void free_socket(void * data) {
	free(*(SOCKET**)data);
}

bool compare_node_with_topic(void *listNode, void* element2) {

	ListNode* node = (ListNode*)listNode;
	char find_topic = *(char*)element2;

	TopicContent *current_topic_content = (TopicContent*)node->data;
	char current_topic = current_topic_content->topic;

	if (current_topic == find_topic) {
		return true;
	}
	else {
		return false;
	}
}

#pragma endregion

#pragma region THREAD_FUNCTIONS

DWORD WINAPI accept_publisher(LPVOID lpParam) {
	List* topic_contents = (List*)lpParam;
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET* acceptedSocket;
	start_listening(&listenSocket, LISTEN_PUBLISHER_PORT);
	printf("PubSubEngine is ready to accept publishers.\n");

	while (true) {
		SOCKET socket = INVALID_SOCKET;
		socket = accept(listenSocket, NULL, NULL);
		if (socket != INVALID_SOCKET)
		{
			acceptedSocket = (SOCKET*)malloc(sizeof(SOCKET));
			memcpy(acceptedSocket, &socket, sizeof(SOCKET));
			DWORD listen_publisher_id;
			ParamStruct param;
			param.socket = acceptedSocket;
			param.topic_contents = topic_contents;
			HANDLE listen_publisher_handle = CreateThread(NULL, 0, &listen_publisher, &param, 0, &listen_publisher_id);

			// TODO: put SOCKET and HANDLER in container

		}
	}

	return 0;
}

DWORD WINAPI listen_publisher(LPVOID lpParam) {
	ParamStruct* param = (ParamStruct*)lpParam;
	SOCKET* socket = param->socket;
	List* topic_contents = param->topic_contents;
	waitForMessage(socket, DEFAULT_BUFLEN, topic_contents);
	return 0;
}

DWORD WINAPI accept_subscriber(LPVOID lpParam) {
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET* acceptedSocket;
	List *topic_contents = (List*)lpParam;

	start_listening(&listenSocket, LISTEN_SUBSCRIBER_PORT);
	printf("PubSubEngine is ready to accept subscribers.\n");

	SOCKET socket;

	while (true) {
		socket = INVALID_SOCKET;
		socket = accept(listenSocket, NULL, NULL);
		if (socket != INVALID_SOCKET) {

			acceptedSocket = (SOCKET*)malloc(sizeof(SOCKET));
			memcpy(acceptedSocket, &socket, sizeof(SOCKET));

			bool success = receiveTopic(acceptedSocket, 2, topic_contents);

		}
	}

	return 0;
}

DWORD WINAPI consume_messages(LPVOID lpParam) {
	return 0;
}

#pragma endregion

#pragma region PUBLISHER

void waitForMessage(SOCKET * socket, unsigned buffer_size, List* topic_contents)
{
	int iResult;
	char *recvbuf = (char*)malloc(buffer_size);
	//set parameter for NonBlocking mode
	setNonBlockingMode(socket);

	printf("\n Waiting for messages...\n");
	do
	{
		iResult = ready_for_receive(socket);

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
			char topic = recvbuf[0];
			char message = recvbuf[1];
			bool success = pushMessage(topic, message, topic_contents);
			if (!success) {
				create_topic(topic_contents, topic);
				pushMessage(topic, message, topic_contents);
			}
			printf("Message received on topic %c: [%c]", topic, message);
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

bool pushMessage(char topic, char message, List* topic_contents) {
	return false;
}

void create_topic(List* topic_contents, char topic) {

}

#pragma endregion

#pragma region SUBSCRIBER

bool receiveTopic(SOCKET *socket, unsigned buffer_size, List *topic_contents) {
	int iResult;
	char *recvbuf = (char*)malloc(buffer_size);
	//set parameter for NonBlocking mode
	setNonBlockingMode(socket);

	printf("\n Waiting for messages...\n");
	do
	{
		iResult = ready_for_receive(socket);

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

			bool success = push_socket_on_topic(topic_contents, socket, recvbuf[0]);

			//TODO if TRUE -> send OK, else FAIL
			char c;
			c = success ? SUBSCRIBE_SUCCESS : SUBSCRIBE_FAIL;
			//confirmation for subscriber
			if (success)
				printf("New subscriber %d for topic: %s.\n", (int)*socket, recvbuf);
			else {
				printf("Subscriber %d fail while trying to subscribe for topic: %s.\n", (int)*socket, recvbuf);
			}
			send(*socket, &c, 1, 0);

			return true;
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(*socket);
			//exit(EXIT_FAILURE);
			return false;
		}
		else
		{
			// there was an error during recv
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(*socket);
			//exit(EXIT_FAILURE);
			return false;
		}
		// here is where server shutdown loguc could be placed

	} while (1);
}

bool push_socket_on_topic(List *topic_contents, SOCKET *socket, char topic) {

	//return node from list where is comparator return true
	ListNode *finded_content = (ListNode*)list_find(topic_contents, &topic, compare_node_with_topic);

	if (finded_content == NULL) {
		return false;
	}

	//extract data from finded content
	TopicContent *topic_content = (TopicContent*)finded_content->data;


	list_append(&topic_content->sockets, socket);

	return true;
}

#pragma endregion
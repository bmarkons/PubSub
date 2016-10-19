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

bool receive(SOCKET* socket, char* recvbuf) {
	int topic_length = 1;
	int iResult;
	int total_received = 0;
	do {
		iResult = recv(*socket, recvbuf + total_received, topic_length - total_received, 0);
		total_received += iResult;
	} while (total_received < topic_length);

	return iResult < 0 ? false : true;
}

void wait_for_message(SOCKET * socket, List* topic_contents, messageHandler message_handler)
{
	set_nonblocking_mode(socket);
	char *recvbuf = (char*)malloc(DEFAULT_BUFLEN);

	printf("\n Waiting for messages...\n");
	do
	{
		bool ready = is_ready_for_receive(socket);
		bool success;
		if (ready) {
			success = receive(socket, recvbuf);
			if (success) {
				message_handler(recvbuf, socket, topic_contents);
			}
			else {
				printf("Error occured while receiving message from socket.\n");
				closesocket(*socket);
				break;
			}
		}
	} while (1);
}

#pragma endregion

#pragma region TOPIC_LIST

void free_topic_content(void * data) {
	free(*(TopicContent**)data);
}

void free_socket(void * data) {
	free(*(SOCKET**)data);
}

bool compare_node_with_topic(ListNode* listNode, void* param) {

	char find_topic = *(char*)param;

	TopicContent *current_topic_content = (TopicContent*)listNode->data;
	char current_topic = current_topic_content->topic;

	if (current_topic == find_topic) {
		return true;
	}
	else {
		return false;
	}
}

bool sendIterator(ListNode *listNode, void* param) {

	char message = *(char*)param;

	SOCKET socket = *(SOCKET*)listNode->data;

	send(socket, &message, 1, 0);

	return true;
}

#pragma endregion

#pragma region PUBLISHER

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

	wait_for_message(socket, topic_contents, unpack_and_push);

	return 0;
}

void unpack_and_push(char* recvbuf, SOCKET* socket, List* topic_contents) {
	char topic;
	char message;

	unpack_message(recvbuf, &topic, &message);
	push_message(topic, message, topic_contents);

	printf("Message pushed on topic %c: [%c]", topic, message);
}

void unpack_message(char* recvbuf, char* topic, char* message) {
	*topic = recvbuf[0];
	*message = recvbuf[1];
}

void push_message(char topic, char message, List* topic_contents) {
	bool success = push_try(topic, message, topic_contents);
	if (!success) {
		create_topic(topic_contents, topic);
		push_try(topic, message, topic_contents);
	}
}

bool push_try(char topic, char message, List* topic_contents) {
	ListNode* node = (ListNode*)list_find(topic_contents, &topic, compare_node_with_topic);

	if (node == NULL) {
		return false;
	}

	TopicContent* topic_content = (TopicContent*)node->data;
	Push(&topic_content->message_buffer, message);

	return true;
}

void create_topic(List* topic_contents, char topic) {
	TopicContent new_topic;
	new_topic.topic = topic;
	list_new(&new_topic.sockets, sizeof(SOCKET), free_socket);
	InitializeBuffer(&new_topic.message_buffer, INIT_BUFFER_SIZE);

	ListNode* node = list_append(topic_contents, &new_topic);

	HANDLE consume_message_handle = CreateThread(NULL, 0, &consume_messages, node->data, 0, NULL);
	// TODO: add handle to HANDLE_LIST
}

#pragma endregion

#pragma region SUBSCRIBER

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

			wait_for_message(acceptedSocket, topic_contents, push_socket_on_topic);
		}
	}

	return 0;
}

DWORD WINAPI consume_messages(LPVOID lpParam) {
	TopicContent *topic_content = (TopicContent*)lpParam;
	char message;
	while (true) {

		bool success = Pop(&topic_content->message_buffer, &message);
		if (success) {
			send_to_sockets(&topic_content->sockets, message);
		}
		Sleep(SERVER_SLEEP_TIME);
	}
	return 0;
}

void push_socket_on_topic(char* recvbuf, SOCKET *socket, List *topic_contents) {
	char topic = recvbuf[0];
	ListNode *finded_content = (ListNode*)list_find(topic_contents, &topic, compare_node_with_topic);

	char response;
	if (finded_content == NULL) {
		response = SUBSCRIBE_FAIL;
		printf("Subscriber %d fail while trying to subscribe for topic: %s.\n", (int)*socket, recvbuf);
	}
	else {
		TopicContent *topic_content = (TopicContent*)finded_content->data;
		list_append(&topic_content->sockets, socket);

		response = SUBSCRIBE_SUCCESS;
		printf("New subscriber %d for topic: %s.\n", (int)*socket, recvbuf);
	}

	send(*socket, &response, 1, 0);
}

void send_to_sockets(List *sockets, char message) {
	list_for_each_param(sockets, sendIterator, &message);
}

#pragma endregion
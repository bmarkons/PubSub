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
	int topic_length = 0;
	int iResult;
	int total_received = 0;
	bool firstRecv = true;
	do {
		if (firstRecv) {
			iResult = recv(*socket, recvbuf + total_received, 1, 0);
			topic_length = recvbuf[0];
			firstRecv = false;
		}
		else {
			iResult = recv(*socket, recvbuf + total_received, DEFAULT_BUFLEN, 0);
			total_received += iResult;
		}
		if (iResult < 0) {
			break;
		}

	} while (total_received < topic_length);

	recvbuf[total_received] = NULL;  //set the end of the string
	return iResult < 0 ? false : true;
}

void wait_for_message(SOCKET * socket, Wrapper* wrapper, messageHandler message_handler) {
	set_nonblocking_mode(socket);
	char *recvbuf = (char*)malloc(DEFAULT_BUFLEN);

	do
	{
		bool ready = is_ready_for_receive(socket);
		bool success;
		if (ready) {
			success = receive(socket, recvbuf);
			if (success) {
				message_handler(recvbuf, socket, wrapper);
			}
			else {
				printf("Error occured while receiving message from socket.\n");
				closesocket(*socket);
				break;
			}
		}
	} while (1);
	free(recvbuf);
}

void send_to_subscriber(SOCKET * socket, TString message)
{
	int data_size;
	char* package = make_data_package(message, &data_size);
	bool success = send_nonblocking(socket, package, data_size);
	if (success) {
		printf("--> Message sent to subcriber : %d\n", *socket);
	}
	else {
		printf("xxx Error occured while sending to subscriber %d.\n", *socket);
	}

}

bool send_nonblocking(SOCKET* socket, char* package, int data_size) {
	set_nonblocking_mode(socket);

	while (true) {
		bool ready = is_ready_for_send(socket);
		bool success;

		if (ready) {
			success = send_all(socket, package, data_size);
			free(package);
			if (!success) {
				closesocket(*socket);
			}
			return success;
		}
	}
}

bool send_all(SOCKET* socket, char *package, int data_size) {
	int package_size = data_size + HEADER_SIZE;
	int iResult;
	int total_sent = 0;
	do {
		iResult = send(*socket, package + total_sent, package_size - total_sent, 0);
		total_sent += iResult;
	} while (total_sent < package_size);

	return iResult == SOCKET_ERROR ? false : true;
}

char* make_data_package(TString message, int* data_size) {
	//int size_of_package = strlen(topic);
	*data_size = message.length + 1;
	char* data_package = (char*)malloc(sizeof(char)*(*data_size + 1));
	data_package[0] = *data_size;
	data_package[1] = message.length;
	memcpy(data_package + 2, message.text, message.length);
	return data_package;
}
#pragma endregion

#pragma region LIST_FUNCTIONS

void free_topic_content(void * data) {
	free(*(TopicContent**)data);
}

void free_socket(void * data) {
	free(*(SOCKET**)data);
}

void free_thread(void * data) {
	free(*(TThread**)data);
}

bool compare_node_with_topic(ListNode* listNode, void* param) {

	TString find_topic = *(TString*)param;

	TopicContent *current_topic_content = (TopicContent*)listNode->data;
	TString current_topic = current_topic_content->topic;

	if (is_equal_string(current_topic, find_topic)) {
		return true;
	}
	else {
		return false;
	}
}

bool sendIterator(ListNode *listNode, void* param) {

	TString message = *(TString*)param;

	SOCKET socket = *(SOCKET*)listNode->data;

	send_to_subscriber(&socket, message);

	return true;
}

bool printID(void *param) {
	TThread thread = *(TThread*)param;

	printf(" - %d :", thread.id);
	DWORD result = WaitForSingleObject(thread.handle, 0);
	char *msg;
	if (result == WAIT_OBJECT_0) {
		// the thread handle is signaled - the thread has terminated
		msg = "TERMINATED";
	}
	else {
		// the thread handle is not signaled - the thread is still alive
		msg = "ALIVE";
	}
	printf(" %s\n", msg);
	return true;
}

ListNode* create_topic(Wrapper* wrapper, TString topic) {
	TopicContent new_topic = initializeTopic(topic);
	ListNode* node = list_append(wrapper->topic_contents, &new_topic);

	DWORD consume_thread_id;
	HANDLE consume_message_handle = CreateThread(NULL, 0, &consume_messages, node->data, 0, &consume_thread_id);

	add_to_thread_list(wrapper->thread_list,            //list
		consume_message_handle,        //handle
		consume_thread_id);           //handle_id

	return node;
}

TopicContent initializeTopic(TString topic) {
	TopicContent new_topic;
	new_topic.topic = topic;
	list_new(&new_topic.sockets, sizeof(SOCKET), free_socket);
	InitializeBuffer(&new_topic.message_buffer, INIT_BUFFER_SIZE);
	return new_topic;
}

bool is_equal_string(TString s1, TString s2) {
	if (s1.length != s2.length) {
		return false;
	}
	for (int i = 0; i < s1.length; i++) {
		if (s1.text[i] != s2.text[i]) {
			return false;
		}
	}
	return true;
}

#pragma endregion

#pragma region PUBLISHER

DWORD WINAPI accept_publisher(LPVOID lpParam) {

	Wrapper* wrapper = (Wrapper*)lpParam;

	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET* acceptedSocket;
	start_listening(&listenSocket, LISTEN_PUBLISHER_PORT);
	printf("PubSubEngine is ready to accept publishers.\n");

	while (true) {
		SOCKET socket = INVALID_SOCKET;
		socket = accept(listenSocket, NULL, NULL);
		if (socket != INVALID_SOCKET)
		{
			printf("[Publisher] Connected.\n");
			acceptedSocket = (SOCKET*)malloc(sizeof(SOCKET));
			memcpy(acceptedSocket, &socket, sizeof(SOCKET));
			DWORD listen_publisher_id;
			ParamStruct param;
			param.socket = acceptedSocket;
			param.wrapper = wrapper;

			HANDLE listen_publisher_handle = CreateThread(NULL, 0, &listen_publisher, &param, 0, &listen_publisher_id);

			add_to_thread_list(wrapper->thread_list,            //list
				listen_publisher_handle,        //handle
				listen_publisher_id);           //handle_id
		}
	}

	return 0;
}

DWORD WINAPI listen_publisher(LPVOID lpParam) {
	ParamStruct* param = (ParamStruct*)lpParam;
	SOCKET* socket = param->socket;

	Wrapper* wrapper = param->wrapper;

	wait_for_message(socket, wrapper, unpack_and_push);

	return 0;
}

void unpack_and_push(char* recvbuf, SOCKET* socket, Wrapper* wrapper) {
	TString topic;
	TString message;

	unpack_message(recvbuf, &topic, &message);
	push_message(topic, message, wrapper);

	printf("[Publisher] New message on topic %s: [%s]\n", topic.text, message.text);

}

void unpack_message(char* recvbuf, TString *topic, TString *message) {

	topic->length = recvbuf[0];
	topic->text = (char*)calloc(topic->length + 1, sizeof(char));
	memcpy(topic->text, recvbuf + 2, topic->length);

	message->length = recvbuf[1];
	message->text = (char*)calloc(message->length + 1, sizeof(char));
	memcpy(message->text, recvbuf + 2 + topic->length, message->length);

}

void push_message(TString topic, TString message, Wrapper* wrapper) {
	bool success = push_try(topic, message, wrapper->topic_contents);
	if (!success) {
		create_topic(wrapper, topic);
		push_try(topic, message, wrapper->topic_contents);
	}
}

bool push_try(TString topic, TString message, List* topic_contents) {
	ListNode* node = (ListNode*)list_find(topic_contents, &topic, compare_node_with_topic);

	if (node == NULL) {
		return false;
	}

	TopicContent* topic_content = (TopicContent*)node->data;
	Push(&topic_content->message_buffer, message);

	return true;
}

#pragma endregion

#pragma region SUBSCRIBER

DWORD WINAPI accept_subscriber(LPVOID lpParam) {

	Wrapper *wrapper = (Wrapper*)lpParam;

	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET* acceptedSocket;

	start_listening(&listenSocket, LISTEN_SUBSCRIBER_PORT);
	printf("PubSubEngine is ready to accept subscribers.\n");

	SOCKET socket;

	while (true) {
		socket = INVALID_SOCKET;
		socket = accept(listenSocket, NULL, NULL);
		if (socket != INVALID_SOCKET) {
			printf("[Subscriber] Connected.\n");
			acceptedSocket = (SOCKET*)malloc(sizeof(SOCKET));
			memcpy(acceptedSocket, &socket, sizeof(SOCKET));

			DWORD listen_subscriber_id;
			ParamStruct param;
			param.socket = acceptedSocket;
			param.wrapper = wrapper;

			HANDLE listen_subscriber_handle = CreateThread(NULL, 0, &listen_subscriber, &param, 0, &listen_subscriber_id);

			add_to_thread_list(wrapper->thread_list,            //list
				listen_subscriber_handle,        //handle
				listen_subscriber_id);           //handle_id
		}
	}

	return 0;
}

DWORD WINAPI listen_subscriber(LPVOID lpParam) {
	ParamStruct* param = (ParamStruct*)lpParam;
	SOCKET* socket = param->socket;
	Wrapper* wrapper = param->wrapper;

	wait_for_message(socket, wrapper, push_socket_on_topic);

	return 0;
}

DWORD WINAPI consume_messages(LPVOID lpParam) {
	TopicContent *topic_content = (TopicContent*)lpParam;
	TString message;
	while (true) {

		bool success = Pop(&topic_content->message_buffer, &message);
		if (success) {
			printf("Sending message [%s] to all subscribers on topic '%s'...\n", message.text, topic_content->topic.text);
			send_to_sockets(&topic_content->sockets, message);
		}
		Sleep(SERVER_SLEEP_TIME);
	}
	return 0;
}

TString unpack_topic(char* recvbuf) {
	TString topic;
	topic.length = recvbuf[0];
	topic.text = (char*)calloc(topic.length + 1, sizeof(char));
	memcpy(topic.text, recvbuf + 1, topic.length);
	return topic;
}

void push_socket_on_topic(char* recvbuf, SOCKET *socket, Wrapper *wrapper) {

	TString topic = unpack_topic(recvbuf);
	ListNode *finded_content = (ListNode*)list_find(wrapper->topic_contents, &topic, compare_node_with_topic);

	if (finded_content == NULL) {
		finded_content = create_topic(wrapper, topic);
	}

	TopicContent *topic_content = (TopicContent*)finded_content->data;
	list_append(&topic_content->sockets, socket);

	printf("[Subscriber] New subscription on topic: %s.\n", topic.text);

	int data_size;
	TString succes_message;
	succes_message.length = 1;
	succes_message.text = (char*)calloc(1, sizeof(char));
	succes_message.text[0] = SUBSCRIBE_SUCCESS;

	char* package = make_data_package(succes_message, &data_size);
	bool success = send_nonblocking(socket, package, data_size);
	if (!success) {
		printf("Error occured while sending subscription confirmation.\n");
	}

	free(succes_message.text);
}

void send_to_sockets(List *sockets, TString message) {
	list_for_each_param(sockets, sendIterator, &message);
}

#pragma endregion

#pragma region THREAD_COLLECTOR
DWORD WINAPI thread_collector(LPVOID lpParam) {

	Wrapper *wrapper = (Wrapper*)lpParam;

	while (true) {
		printf("\nBefore");
		print_all_threads(wrapper->thread_list);

		find_and_remove_terminated(wrapper->thread_list);

		printf("\nAfter");
		print_all_threads(wrapper->thread_list);
		Sleep(2000);
	}
	return 0;
}

void add_to_thread_list(List* thread_list, HANDLE handle, DWORD handle_id) {
	TThread thread;
	thread.handle = handle;
	thread.id = handle_id;
	list_append(thread_list, &thread);
}

void find_and_remove_terminated(List* thread_list) {
	EnterCriticalSection(&thread_list->cs);

	ListNode *node = thread_list->head;
	ListNode *previous = NULL;
	DWORD result;
	TThread *thread;

	while (node != NULL) {
		thread = (TThread*)node->data;
		result = WaitForSingleObject(thread->handle, 0);
		if (result == WAIT_OBJECT_0) {
			/*shutdown terminated thread*/
			CloseHandle(thread->handle);

			if (previous == NULL) {
				thread_list->head = node->next;
				free(node);
				thread_list->logicalLength--;
				node = thread_list->head;
			}
			else {
				previous->next = node->next;

				if (thread_list->tail == node) {
					thread_list->tail = previous;
				}
				free(node);
				thread_list->logicalLength--;
				node = previous->next;
			}
		}
		else {
			previous = node;
			node = node->next;
		}
	}
	LeaveCriticalSection(&thread_list->cs);
}

void print_all_threads(List* thread_list) {
	printf("\n ***Print threads***\n");
	printf(" -Number of threads: %d\n", thread_list->logicalLength);
	list_for_each(thread_list, printID);
}
#pragma endregion
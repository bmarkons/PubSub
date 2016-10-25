#include "stdafx.h"

#pragma region LIST_FUNCTIONS

void free_topic_content(void * data) {
	TopicContent* topic_content = (TopicContent*)data;
	list_destroy(&topic_content->sockets);
	free(topic_content->message_buffer.buffer);
	free(topic_content->topic.array);
}

void free_socket(void * data) {
	free(*(SOCKET**)data);
}

void free_thread(void * data) {
	TThread* thread = (TThread*)data;
	TerminateThread(thread->handle, 0);
	CloseHandle(thread->handle);
}

bool compare_node_with_topic(ListNode* listNode, void* param) {

	ByteArray find_topic = *(ByteArray*)param;

	TopicContent *current_topic_content = (TopicContent*)listNode->data;
	ByteArray current_topic = current_topic_content->topic;

	if (is_equal_string(current_topic, find_topic)) {
		return true;
	}
	else {
		return false;
	}
}

bool sendIterator(ListNode *listNode, void* param) {

	ByteArray message = *(ByteArray*)param;

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

ListNode* create_topic(Wrapper* wrapper, ByteArray topic) {
	TopicContent new_topic = initializeTopic(topic);
	ListNode* node = list_append(wrapper->topic_contents, &new_topic);

	DWORD consume_thread_id;
	HANDLE consume_message_handle = CreateThread(NULL, 0, &consume_messages, node->data, 0, &consume_thread_id);

	add_to_thread_list(wrapper->thread_list,            //list
		consume_message_handle,        //handle
		consume_thread_id);           //handle_id

	return node;
}

TopicContent initializeTopic(ByteArray topic) {
	TopicContent new_topic;
	new_topic.topic = topic;
	list_new(&new_topic.sockets, sizeof(SOCKET), NULL);
	InitializeBuffer(&new_topic.message_buffer, INIT_BUFFER_SIZE);
	return new_topic;
}

bool is_equal_string(ByteArray s1, ByteArray s2) {
	if (s1.size != s2.size) {
		return false;
	}
	for (int i = 0; i < s1.size; i++) {
		if (s1.array[i] != s2.array[i]) {
			return false;
		}
	}
	return true;
}

#pragma endregion

#pragma region PUBLISHER

DWORD WINAPI accept_publisher(LPVOID lpParam) {

	Wrapper* wrapper = (Wrapper*)lpParam;
	start_listening(wrapper->accepting_publisher, LISTEN_PUBLISHER_PORT);
	printf("PubSubEngine is ready to accept publishers.\n");

	while (true) {
		SOCKET socket = INVALID_SOCKET;
		socket = accept(*wrapper->accepting_publisher, NULL, NULL);
		if (socket != INVALID_SOCKET)
		{
			printf("[Publisher] Connected.\n");
			DWORD listen_publisher_id;
			ParamStruct param;
			param.socket = socket;
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
	SOCKET socket = param->socket;

	void* wrapper = param->wrapper;

	wait_for_message(&socket, wrapper, false, unpack_and_push);

	return 0;
}

void unpack_and_push(SOCKET* socket, char* recvbuf, void* param) {
	Wrapper* wrapper = (Wrapper*)param;

	ByteArray topic;
	ByteArray message;

	unpack_message(recvbuf, &topic, &message);
	free(recvbuf);

	//printf("[Publisher] New message on topic %s: [%s]\n", topic.array, message.array);
	push_message(topic, message, wrapper);
}

void unpack_message(char* recvbuf, ByteArray *topic, ByteArray *message) {
	memcpy(&topic->size, recvbuf, sizeof(u_short));
	topic->array = (char*)calloc(topic->size + 1, sizeof(char));
	memcpy(topic->array, recvbuf + 4, topic->size);

	memcpy(&message->size, recvbuf + 2, sizeof(u_short));
	message->array = (char*)calloc(message->size + 1, sizeof(char));
	memcpy(message->array, recvbuf + 4 + topic->size, message->size);
}

void push_message(ByteArray topic, ByteArray message, Wrapper* wrapper) {
	bool success = push_try(topic, message, wrapper->topic_contents);
	if (!success) {
		create_topic(wrapper, topic);
		push_try(topic, message, wrapper->topic_contents);
	}
	free(topic.array);
}

bool push_try(ByteArray topic, ByteArray message, List* topic_contents) {
	ListNode* node = (ListNode*)list_find(topic_contents, &topic, compare_node_with_topic);

	if (node == NULL) {
		return false;
	}

	TopicContent* topic_content = (TopicContent*)node->data;
	
	bool success = false;
	while (!success) {
		success = Push(&topic_content->message_buffer, message);
		if (!success) {
			Sleep(DEFAULT_SLEEP_TIME);
		}
	}

	return true;
}

#pragma endregion

#pragma region SUBSCRIBER

DWORD WINAPI accept_subscriber(LPVOID lpParam) {
	Wrapper *wrapper = (Wrapper*)lpParam;

	start_listening(wrapper->accepting_subscriber, LISTEN_SUBSCRIBER_PORT);
	printf("PubSubEngine is ready to accept subscribers.\n");

	SOCKET socket;

	while (true) {
		socket = INVALID_SOCKET;
		socket = accept(*wrapper->accepting_subscriber, NULL, NULL);
		if (socket != INVALID_SOCKET) {
			printf("[Subscriber] Connected.\n");
			DWORD listen_subscriber_id;
			ParamStruct param;
			param.socket = socket;
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
	SOCKET socket = param->socket;
	void* wrapper = param->wrapper;

	wait_for_message(&socket, wrapper, true, push_socket_on_topic);

	return 0;
}

DWORD WINAPI consume_messages(LPVOID lpParam) {
	TopicContent *topic_content = (TopicContent*)lpParam;
	ByteArray message;
	int num_of_removed = 0;
	clock_t start = clock();
	clock_t now;
	float delta_time = 0;
	while (true) {
		now = clock();
		delta_time = (float)(now - start) / CLOCKS_PER_SEC;
		num_of_removed = 0;
		if (delta_time > 2) {
			num_of_removed = clean_from_closed_sockets(&topic_content->sockets);
			start = now;
		}

		if (num_of_removed != 0)
			printf("Removed %d closed socket on %s topic\n", num_of_removed, topic_content->topic.array);

		bool success = Pop(&topic_content->message_buffer, &message);
		if (success) {
			//printf("Sending message [%s] to all subscribers on topic '%s'...\n", message.array, topic_content->topic.array);
			send_to_sockets(&topic_content->sockets, message);
			free(message.array);
		}

		if (CONSUME_MESSAGES_SLEEP != 0) {
			Sleep(CONSUME_MESSAGES_SLEEP);
		}
	}
	return 0;
}
/*return number of deleted socket from list*/
int clean_from_closed_sockets(List* sockets) {

	int count = 0;//number of deleted sockets
	if (&sockets->cs == NULL) {
		return 0;
	}
	EnterCriticalSection(&sockets->cs);
	if (&sockets->cs == NULL) {
		return 0;
	}
	ListNode *node = sockets->head;
	ListNode *previous = NULL;
	SOCKET *socket;
	int return_code;//return code from function is_ready_for_send

	while (node != NULL) {
		socket = (SOCKET*)node->data;

		int ready = is_ready_for_send(socket);

		if (ready == SOCKET_ERROR) {

			closesocket(*socket);
			count++;

			if (previous == NULL) {
				sockets->head = node->next;
				free(node);
				sockets->logicalLength--;
				node = sockets->head;
			}
			else {
				previous->next = node->next;

				if (sockets->tail == node) {
					sockets->tail = previous;
				}
				free(node);
				sockets->logicalLength--;
				node = previous->next;
			}
		}
		else {
			previous = node;
			node = node->next;
		}
	}
	LeaveCriticalSection(&sockets->cs);
	return count;


}

ByteArray unpack_topic(char* recvbuf) {
	ByteArray topic;
	memcpy(&topic.size, recvbuf, sizeof(u_short));
	topic.array = (char*)calloc(topic.size + 1, sizeof(char));
	memcpy(topic.array, recvbuf + 2, topic.size);
	return topic;
}

void push_socket_on_topic(SOCKET *socket, char* recvbuf, void* param) {
	Wrapper* wrapper = (Wrapper*)param;

	ByteArray topic = unpack_topic(recvbuf);
	free(recvbuf);
	ListNode *finded_content = (ListNode*)list_find(wrapper->topic_contents, &topic, compare_node_with_topic);

	if (finded_content == NULL) {
		finded_content = create_topic(wrapper, topic);
	}

	TopicContent *topic_content = (TopicContent*)finded_content->data;
	list_append(&topic_content->sockets, socket);

	printf("[Subscriber] New subscription on topic: %s.\n", topic.array);

	int data_size;
	ByteArray succes_message;
	succes_message.size = 1;
	succes_message.array = (char*)calloc(1, sizeof(char));
	succes_message.array[0] = SUBSCRIBE_SUCCESS;

	ByteArray package = make_package(succes_message);
	bool success = send_nonblocking(socket, package);
	if (!success) {
		printf("Error occured while sending subscription confirmation.\n");
	}

	free(succes_message.array);
}

void send_to_sockets(List *sockets, ByteArray message) {
	list_for_each_param(sockets, sendIterator, &message);
}

void send_to_subscriber(SOCKET * socket, ByteArray message)
{
	ByteArray package = make_package(message);
	bool success = send_nonblocking(socket, package);
	if (success) {
		//printf("--> Message sent to subcriber : %d\n", *socket);
	}
	else {
		printf("xxx Error occured while sending to subscriber %d.\n", *socket);
	}

}

#pragma endregion

#pragma region THREAD_COLLECTOR
DWORD WINAPI thread_collector(LPVOID lpParam) {

	Wrapper *wrapper = (Wrapper*)lpParam;
	int result;
	while (true) {

		result = WaitForSingleObject(GetCurrentThread(), 0);
		if (result == WAIT_OBJECT_0) {
			CloseHandle(GetCurrentThread());
			break;
		}
		/*	printf("\nBefore");
			print_all_threads(wrapper->thread_list);*/
		find_and_remove_terminated(wrapper->thread_list);

		//printf("\nAfter");
		//print_all_threads(wrapper->thread_list);
		Sleep(THREAD_COLLECTOR_SLEEP);
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
				free(node->data);
				free(node);
				thread_list->logicalLength--;
				node = thread_list->head;
			}
			else {
				previous->next = node->next;

				if (thread_list->tail == node) {
					thread_list->tail = previous;
				}
				free(node->data);
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

ByteArray make_package(ByteArray message) {
	ByteArray package;
	package.size = message.size + 4;
	package.array = (char*)malloc(package.size * sizeof(char));

	u_short main_header = package.size - 2;
	u_short message_header = message.size;
	memcpy(package.array, &main_header, sizeof(u_short));
	memcpy(package.array + 2, &message_header, sizeof(u_short));
	memcpy(package.array + 4, message.array, message.size);

	return package;
}

#pragma once

#define INIT_BUFFER_SIZE 30
#define MAIN_HEADER_SIZE 2

typedef struct _topic_content {
	ByteArray topic;
	List sockets;
	TBuffer message_buffer;
}TopicContent;

typedef struct _thread {
	DWORD id;
	HANDLE handle;
}TThread;

typedef struct _wrapper {
	List* thread_list;
	List* topic_contents;
	SOCKET* accepting_publisher;
	SOCKET* accepting_subscriber;
}Wrapper;

typedef struct _param_struct {
	SOCKET socket;
	Wrapper* wrapper;
}ParamStruct;

#pragma region LIST_FUNCTIONS
void free_topic_content(void * data);
void free_socket(void * data);
void free_thread(void * data);
bool compare_node_with_topic(ListNode *listNode, void* param);
bool sendIterator(ListNode *listNode, void* param);
bool printID(void *param);
ListNode* create_topic(Wrapper* wrapper, ByteArray topic);
TopicContent initializeTopic(ByteArray topic);
bool is_equal_string(ByteArray s1, ByteArray s2);
#pragma endregion


#pragma region PUBLISHER
DWORD WINAPI accept_publisher(LPVOID lpParam);
DWORD WINAPI listen_publisher(LPVOID lpParam);
void unpack_and_push(SOCKET* socket, char* recvbuf, void* param);
void unpack_message(char* recvbuf, ByteArray* topic, ByteArray* message);
void push_message(ByteArray topic, ByteArray message, Wrapper* wrapper);
bool push_try(ByteArray topic, ByteArray message, List* topic_contents);
#pragma endregion


#pragma region SUBSCRIBER
DWORD WINAPI accept_subscriber(LPVOID lpParam);
DWORD WINAPI listen_subscriber(LPVOID lpParam);
DWORD WINAPI consume_messages(LPVOID lpParam);
void push_socket_on_topic(SOCKET *socket, char* recvbuf, void* param);
ByteArray unpack_topic(char* recvbuf);
void send_to_sockets(List *sockets, ByteArray message);
int clean_from_closed_sockets(List* sockets);
void send_to_subscriber(SOCKET * socket, ByteArray message);
#pragma endregion

#pragma region THREAD_COLLECTOR
DWORD WINAPI thread_collector(LPVOID lpParam);
void add_to_thread_list(List* thread_list, HANDLE handle, DWORD handle_id);
void find_and_remove_terminated(List* thread_list);
void print_all_threads(List* thread_list);
#pragma endregion

ByteArray make_package(ByteArray message);
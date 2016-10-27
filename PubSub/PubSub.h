#pragma once

#define INIT_BUFFER_SIZE 30
#define MAIN_HEADER_SIZE 2
#define THREAD_COLLECTOR_SLEEP 2000
#define CONSUME_MESSAGES_SLEEP 50

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
	bool is_app_end;
}Wrapper;


typedef struct _param_struct {
	Wrapper* wrapper;
	void* param1;	
}ParamStruct;

typedef struct _param_struct1 {
	Wrapper* wrapper;
	SOCKET socket;
}ParamStruct1;

#pragma region PUBLISHER
/**
	Thread function for accepting incoming publishers

	@param lpParam
	@return handle
*/
DWORD WINAPI accept_publisher(LPVOID lpParam);
/**
	Thread function for listening incoming messages from certain publisher

	@param lpParam
	@return handle
*/
DWORD WINAPI listen_publisher(LPVOID lpParam);
/**
	Handler for received message

	@param socket
	@param recvbuf
	@param param
*/
void unpack_and_push(SOCKET* socket, char* recvbuf, void* param);
/**
	Thread function for accepting incoming publishers

	@param
*/
void unpack_message(char* recvbuf, ByteArray* topic, ByteArray* message);
/**
	Push message on given topic

	@param topic
	@param message
	@param wrapper
*/
void push_message(ByteArray topic, ByteArray message, Wrapper* wrapper);
/**
	Try to push message if given topic exists

	@param topic
	@param message
	@param topic_contents
*/
bool push_try(ByteArray topic, ByteArray message, List* topic_contents);
#pragma endregion


#pragma region SUBSCRIBER
/**
	Thread function for accepting incoming subscribers

	@param lpParam
	@return handle
*/
DWORD WINAPI accept_subscriber(LPVOID lpParam);
/**
	Thread function for listening incoming subscription for certain subscriber

	@param lpParam
	@return handle
*/
DWORD WINAPI listen_subscriber(LPVOID lpParam);
/**
	Thread function for consuming messages from certain topic

	@param lpParam
	@return handle
*/
DWORD WINAPI consume_messages(LPVOID lpParam);
/**
	Handler for received message

	@param socket
	@param recvbuf
	@param param
*/
void push_socket_on_topic(SOCKET *socket, char* recvbuf, void* param);
/**
	Extract topic from given byte array

	@param recvbuf
	@return topic ByteArray
*/
ByteArray unpack_topic(char* recvbuf);
/**
	Send message to given list of sockets

	@param sockets
	@param message
*/
void send_to_sockets(List *sockets, ByteArray message);
/**
	Collect closed sockets periodically

	@param recvbuf
	@return topic ByteArray
*/
int clean_from_closed_sockets(List* sockets);
/**
	Send message to given socket

	@param socket
	@param message
*/
void send_to_subscriber(SOCKET * socket, ByteArray message);
#pragma endregion

#pragma region THREAD_COLLECTOR
/**
	Collect closed threads periodically

	@param lpParam
*/
DWORD WINAPI thread_collector(LPVOID lpParam);
/**
	Add thread handle to list of all thread handles

	@param thread_list
	@param handle
	@param handle_id
*/
void add_to_thread_list(List* thread_list, HANDLE handle, DWORD handle_id);
/**
	Go through thread list and find closed threads and remove them

	@param thread_list
*/
void find_and_remove_terminated(List* thread_list);
/**
	Print list of threads in thread list

	@param thread list
*/
void print_all_threads(List* thread_list);
#pragma endregion

ByteArray make_package(ByteArray message);

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
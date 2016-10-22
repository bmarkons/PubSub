#pragma once

#define SUBSCRIBE_SUCCESS 5
#define SUBSCRIBE_FAIL 6
#define SERVER_SLEEP_TIME 50
#define DEFAULT_BUFLEN 10
#define INIT_BUFFER_SIZE 30
#define HEADER_SIZE 1



typedef struct _topic_content {
	TString topic;
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
}Wrapper;

typedef struct _param_struct {
	SOCKET* socket;
	Wrapper* wrapper;
}ParamStruct;

typedef void(*messageHandler)(char*, SOCKET*, Wrapper*);

#pragma region SOCKETS
void InitializeWindowsSockets();
void set_nonblocking_mode(SOCKET* socket);
bool is_ready_for_receive(SOCKET* socket);
bool is_ready_for_send(SOCKET * socket, int *return_code);
void start_listening(SOCKET* listenSocket, char* port);
bool receive(SOCKET* socket, char* recvbuf);
void wait_for_message(SOCKET * socket, Wrapper* wrapper, messageHandler message_handler);
void send_to_subscriber(SOCKET * socket, TString message);
bool send_nonblocking(SOCKET* socket, char* package, int data_size);
bool send_all(SOCKET* socket, char *package, int data_size);
char* make_data_package(TString message, int* data_size);
#pragma endregion


#pragma region LIST_FUNCTIONS
void free_topic_content(void * data);
void free_socket(void * data);
void free_thread(void * data);
bool compare_node_with_topic(ListNode *listNode, void* param);
bool sendIterator(ListNode *listNode, void* param);
bool printID(void *param);
ListNode* create_topic(Wrapper* wrapper, TString topic);
TopicContent initializeTopic(TString topic);
bool is_equal_string(TString s1, TString s2);
#pragma endregion


#pragma region PUBLISHER
DWORD WINAPI accept_publisher(LPVOID lpParam);
DWORD WINAPI listen_publisher(LPVOID lpParam);
void unpack_and_push(char* recvbuf, SOCKET* socket, Wrapper* wrapper);
void unpack_message(char* recvbuf, TString* topic, TString* message);
void push_message(TString topic, TString message, Wrapper* wrapper);
bool push_try(TString topic, TString message, List* topic_contents);
#pragma endregion


#pragma region SUBSCRIBER
DWORD WINAPI accept_subscriber(LPVOID lpParam);
DWORD WINAPI listen_subscriber(LPVOID lpParam);
DWORD WINAPI consume_messages(LPVOID lpParam);
void push_socket_on_topic(char* recvbuf, SOCKET *socket, Wrapper *wrapper);
TString unpack_topic(char* recvbuf);
void send_to_sockets(List *sockets, TString message);
int clean_from_closed_sockets(List* sockets);
#pragma endregion

#pragma region THREAD_COLLECTOR
DWORD WINAPI thread_collector(LPVOID lpParam);
void add_to_thread_list(List* thread_list, HANDLE handle, DWORD handle_id);
void find_and_remove_terminated(List* thread_list);
void print_all_threads(List* thread_list);
#pragma endregion
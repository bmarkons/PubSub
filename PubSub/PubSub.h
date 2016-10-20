#pragma once

#define SUBSCRIBE_SUCCESS 5
#define SUBSCRIBE_FAIL 6
#define SERVER_SLEEP_TIME 50
#define DEFAULT_BUFLEN 10
#define INIT_BUFFER_SIZE 30
#define HEADER_SIZE 1

typedef struct _topic_content {
	char topic;
	List sockets;
	TBuffer message_buffer;
}TopicContent;

typedef struct _param_struct {
	SOCKET* socket;
	List* topic_contents;
}ParamStruct;

typedef void(*messageHandler)(char*, SOCKET*, List*);

#pragma region SOCKETS
void InitializeWindowsSockets();
void set_nonblocking_mode(SOCKET* socket);
bool is_ready_for_receive(SOCKET* socket);
void start_listening(SOCKET* listenSocket, char* port);
bool receive(SOCKET* socket, char* recvbuf);
void wait_for_message(SOCKET * socket, List* topic_contents, messageHandler message_handler);
void send_to_subscriber(SOCKET * socket, char message);
bool send_nonblocking(SOCKET* socket, char* package, int data_size);
bool send_all(SOCKET* socket, char *package, int data_size);
char* make_data_package(char message, int* data_size);
bool is_ready_for_send(SOCKET * socket);
#pragma endregion


#pragma region TOPIC_LIST
void free_topic_content(void * data);
void free_socket(void * data);
bool compare_node_with_topic(ListNode *listNode, void* param);
bool sendIterator(ListNode *listNode, void* param);
#pragma endregion


#pragma region PUBLISHER
DWORD WINAPI accept_publisher(LPVOID);
DWORD WINAPI listen_publisher(LPVOID lpParam);
void unpack_and_push(char* recvbuf, SOCKET* socket, List* topic_contents);
void unpack_message(char* recvbuf, char* topic, char* message);
void push_message(char topic, char message, List* topic_contents);
bool push_try(char topic, char message, List* topic_contents);
void create_topic(List* topic_contents, char topic);
#pragma endregion


#pragma region SUBSCRIBER
DWORD WINAPI accept_subscriber(LPVOID);
DWORD WINAPI listen_subscriber(LPVOID lpParam);
DWORD WINAPI consume_messages(LPVOID lpParam);
void push_socket_on_topic(char* recvbuf, SOCKET *socket, List *topic_contents);
void send_to_sockets(List *sockets, char message);
#pragma endregion
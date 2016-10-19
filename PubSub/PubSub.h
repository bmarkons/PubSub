#pragma once

#define SUBSCRIBE_SUCCESS 5
#define SUBSCRIBE_FAIL 6
#define SERVER_SLEEP_TIME 50
#define DEFAULT_BUFLEN 512
#define INIT_BUFFER_SIZE 30

typedef struct _topic_content {
	char topic;
	List sockets;
	TBuffer message_buffer;
}TopicContent;

typedef struct _param_struct {
	SOCKET* socket;
	List* topic_contents;
}ParamStruct;

#pragma region TOPIC_LIST
void free_topic_content(void * data);
void free_socket(void * data);
bool compare_node_with_topic(void *listNode, void* element2);
#pragma endregion

#pragma region THREAD_FUNCTIONS
DWORD WINAPI accept_publisher(LPVOID);
DWORD WINAPI accept_subscriber(LPVOID);
DWORD WINAPI listen_publisher(LPVOID lpParam);
DWORD WINAPI consume_messages(LPVOID lpParam);
#pragma endregion

#pragma region SOCKETS
void InitializeWindowsSockets();
void setNonBlockingMode(SOCKET* socket);
int ready_for_receive(SOCKET* socket);
void start_listening(SOCKET* listenSocket, char* port);
#pragma endregion

#pragma region PUBLISHER
void waitForMessage(SOCKET*, unsigned, List*);
void create_topic(List* topic_contents, char topic);
bool pushMessage(char topic, char message, List* topic_contents);
#pragma endregion

#pragma region SUBSCRIBER
bool push_socket_on_topic(List *topic_contents, SOCKET *socket, char topic);
bool receiveTopic(SOCKET *socket, unsigned buffer_size, List *topic_contents);
#pragma endregion
#pragma once

#define SUBSCRIBE_SUCCESS 5
#define SUBSCRIBE_FAIL 6
#define SERVER_SLEEP_TIME 50
#define DEFAULT_BUFLEN 512

void InitializeWindowsSockets();
DWORD WINAPI accept_publisher(LPVOID);
DWORD WINAPI accept_subscriber(LPVOID);
DWORD WINAPI listen_publisher(LPVOID lpParam);
void start_listening(SOCKET* listenSocket, char* port);
void waitForMessage(SOCKET*, unsigned);
void setNonBlockingMode(SOCKET* socket);
int Select(SOCKET* socket);
bool push_socket_on_topic(List *topic_contents, SOCKET *socket, char topic);
bool receiveTopic(SOCKET *socket, unsigned buffer_size, List *topic_contents);
void free_topic_content(void * data);
void free_socket(void * data);

typedef struct _topic_content {
	char topic;
	List sockets;
}TopicContent;

TopicContent* list_find(List *list, char topic);
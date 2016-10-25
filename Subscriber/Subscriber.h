#pragma once

#define SUBSCRIBE_SUCCESS 5
#define SUBSCRIBE_FAIL 6
#define SERVER_SLEEP_TIME 50
#define DEFAULT_BUFLEN 100
#define MAIN_HEADER_SIZE 2
#define MAX_INPUT_SIZE 100

typedef struct _byte_array {
	int size;
	char* array;
}ByteArray;

typedef void(*messageHandler)(SOCKET*, char*, void*);

void InitializeWindowsSockets();
void connectToServer(SOCKET* socket, char* ipv4_address, u_int port);
void disconnect(SOCKET*);
void input_message(char* message);
void input_topic(char* topic);
void subscribe(SOCKET* socket, ByteArray topic);
bool send_nonblocking(SOCKET* socket, ByteArray package);
bool send_all(SOCKET* socket, ByteArray package);
ByteArray make_package(ByteArray topic);
void wait_for_message(SOCKET* socket, void* param, bool single_receive, messageHandler message_handler);
void checkConfimation(SOCKET* socket);
void set_nonblocking_mode(SOCKET* socket);
int is_ready_for_receive(SOCKET* socket);
bool is_ready_for_send(SOCKET * socket);
int recv_all(SOCKET* socket, char* recvbuff, int message_length);
bool receive(SOCKET* socket, char** recvbuf);
void subscribing(SOCKET*);
void print_received_message(SOCKET* socket, char* recvbuf, void* param);
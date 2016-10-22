#pragma once

#define SERVER_SLEEP_TIME 50
#define PACKAGE_HEADER_SIZE 6
#define MAX_INPUT_SIZE 100

typedef struct _byte_array {
	int size;
	char* array;
}ByteArray;

void InitializeWindowsSockets();
void connectToServer(SOCKET* socket, char* ipv4_address, u_int port);
void disconnect(SOCKET*);
void publishing_loop(SOCKET* socket);
void input_message(char* message);
void input_topic(char* topic);
void publish(ByteArray message, ByteArray topic, SOCKET* socket);
ByteArray make_package(ByteArray message, ByteArray topic);
bool is_ready_for_send(SOCKET * socket);
void set_nonblocking_mode(SOCKET * socket);
bool send_nonblocking(SOCKET* socket, ByteArray package);
bool send_all(SOCKET* socket, ByteArray package);
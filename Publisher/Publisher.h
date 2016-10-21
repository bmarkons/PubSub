#pragma once

#define SERVER_SLEEP_TIME 50
#define HEADER_SIZE 1
#define MAX_INPUT_SIZE 100
typedef struct _string {
	int length;
	char* text;
}TString;

void InitializeWindowsSockets();
void connectToServer(SOCKET* socket, char* ipv4_address, u_int port);
void disconnect(SOCKET*);
void publishing_loop(SOCKET* socket);
void input_message(char* message);
void input_topic(char* topic);
void publish(char *message, char *topic, SOCKET* socket);
char* make_package(char *message, char *topic, int *data_size);
bool is_ready_for_send(SOCKET * socket);
void set_nonblocking_mode(SOCKET * socket);
bool send_nonblocking(SOCKET* socket, char* package, int data_size);
bool send_all(SOCKET* socket, char *package, int data_size);
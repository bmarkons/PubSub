#pragma once

#define SUBSCRIBE_SUCCESS 5
#define SUBSCRIBE_FAIL 6
#define SERVER_SLEEP_TIME 50
#define DEFAULT_BUFLEN 10

void InitializeWindowsSockets();
void connectToServer(SOCKET* socket, char* ipv4_address, u_int port);
void disconnect(SOCKET*);
void publishing_loop(SOCKET* socket);
void input_message(char* message);
void input_topic(char* topic);
void subscribe(SOCKET* socket, char topic);
char* make_data_package(char topic);
void waitForMessage(SOCKET* socket, unsigned buffer_size);
void checkConfimation(SOCKET* socket);
void set_nonblocking_mode(SOCKET* socket);
bool is_ready_for_receive(SOCKET* socket);
bool is_ready_for_send(SOCKET * socket);
bool Send(SOCKET* socket, char *package);
bool receive(SOCKET* socket, char* recvbuf);
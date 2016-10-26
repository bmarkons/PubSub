#pragma once

#define SUBSCRIBE_SUCCESS 5
#define SUBSCRIBE_FAIL 6
#define DEFAULT_SLEEP_TIME 50
#define DEFAULT_BUFLEN 1024
#define PACKAGE_HEADER_SIZE 6
#define MAIN_HEADER_SIZE 2
#define MAX_INPUT_SIZE 100

typedef struct _byte_array {
	u_short size;
	char* array;
}ByteArray;

typedef void(*messageHandler)(SOCKET*, char*, void*);

/**
	Initialize windows sockets api
*/
void InitializeWindowsSockets();
/**
	Establish connection via given socket on given ipv4 address and port

	@param socket
	@param ipv4_address
	@param port
*/
void connectToServer(SOCKET* socket, char* ipv4_address, u_int port);
/**
	Close socket and exit

	@param socket
*/
void disconnect(SOCKET*);
/**
	Start listen socket on given port

	@param listenSocket
	@param port
*/
void start_listening(SOCKET* listenSocket, char* port);
/**
	Send message in non blocking mode

	@param socket
	@param package
	@return success feedback
*/
bool send_nonblocking(SOCKET* socket, ByteArray package);
/**
	Send message ensuring whole is sent

	@param socket
	@param package
	@return success feedback
*/
bool send_all(SOCKET* socket, ByteArray package);
/**
	Wait for message in loop and handle received message with passed message handler.

	@param socket
	@param param
	@param single_receive - receive only once and return
	@param message_handler - handler for received message
*/
void wait_for_message(SOCKET* socket, void* param, bool single_receive, messageHandler message_handler);
/**
	Set socket in non blocking mode

	@param socket
*/
void set_nonblocking_mode(SOCKET* socket);
/**
	Check if socket is ready for receive

	@param socket
	@return code
*/
int is_ready_for_receive(SOCKET* socket);
/**
	Check if socket is ready for send

	@param socket
	@return code
*/
int is_ready_for_send(SOCKET * socket);
/**
	Receive message from socket ensuring whole message is read

	@param socket
	@param recvbuf
	@param message_length
	@return success code
*/
bool recv_all(SOCKET* socket, char* recvbuf, int message_length);
/**
	Receive message from given socket and stores in buffer

	@param socket
	@param recvbuf
	@return success feedback
*/
bool receive(SOCKET* socket, char** recvbuf);
/**
	receive message part of byte package

	@param socket
	@param recvbuf
	@param message_length
*/
bool receive_message(SOCKET* socket, char** recvbuf, u_short message_length);
/**
	receive header part of byte package

	@param socket
	@param recvbuf
	@param message_length
*/
bool receive_header(SOCKET* socket, OUT u_short* message_length);
/**
	Gets message as string

	@param message
*/
void input_message(char* message);
/**
	Gets topic as string

	@param topic
*/
void input_topic(char* topic);
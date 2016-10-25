#pragma once
/**
	Subscribe method make package from ByteArray and
	send packed packaged to PubSubEngine via nonblocking TCP 

	@param socket - where package will be sent
	@param topic - what is sent to the Engine
*/
void subscribe(SOCKET* socket, ByteArray topic);

/**
	Make package with header added to the array
	Header is added to the front of array and he contain information of size
	of array.

	@param topic 
	@return ByteArray - array with header
*/
ByteArray make_package(ByteArray topic);

/**
	Wait for message from socket

	@param socket
*/
void checkConfimation(SOCKET* socket);

/**
	Method, which is responsible for selecting topic and 
	subscribing on selected topic

	@param socket
*/
void subscribing(SOCKET*);

/**
	Handler for printing on screen about received message
	@param socket
	@param recvbuf
	@param param 
*/
void print_received_message(SOCKET* socket, char* recvbuf, void* param);

/**
	Method checks the received array and print certain message

	@param socket
	@param recvbuf received array
	@param param
*/
void print_subscribe_confirmation(SOCKET* socket, char* recvbuf, void* param);
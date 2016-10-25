#pragma once
/**
	Method, which is responsible for selecting topic and messages
	publishing selected data to PubSubEngine

	@param socket
*/
void publishing_loop(SOCKET* socket);

/**
	Publishing packed data to PubSubEngine

	@param message
	@param topic
	@param socket
*/
void publish(ByteArray message, ByteArray topic, SOCKET* socket);

/**
	Making 2 headers at the beginning of array with size of arrays
	Packing 2 headers,topic and message in one array

	@param message
	@param topic
	@return packed array
*/
ByteArray make_package(ByteArray message, ByteArray topic);
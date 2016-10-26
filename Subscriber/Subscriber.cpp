#include "stdafx.h"

void subscribe(SOCKET * socket, ByteArray topic)
{
	ByteArray package = make_package(topic);

	bool success = send_nonblocking(socket, package);
	if (success) {
		printf("Subscribing on topic '%s'.\n", topic.array);
	}
	else {
		printf("Error occured while subscribing...\n");
	}
}

ByteArray make_package(ByteArray topic) {
	ByteArray package;
	package.size = topic.size + 4;
	package.array = (char*)calloc(package.size + 1, sizeof(char));

	u_short main_header = package.size - 2;
	u_short topic_header = topic.size;
	memcpy(package.array, &main_header, sizeof(u_short));
	memcpy(package.array + 2, &topic_header, sizeof(u_short));
	memcpy(package.array + 4, topic.array, topic.size);

	return package;
}

void checkConfimation(SOCKET *socket) {
	wait_for_message(socket, NULL, true, print_subscribe_confirmation);
}

void subscribing(SOCKET* connectSocket) {
	char command;
	do {
		system("cls");
		char topic[MAX_INPUT_SIZE];
		input_topic(topic);

		//subsribing for specific topic
		ByteArray topic_bytes;
		topic_bytes.size = strlen(topic);
		topic_bytes.array = topic;
		subscribe(connectSocket, topic_bytes);

		//check if server returns a message that everything is ok
		checkConfimation(connectSocket);

		printf("Continue with subscribing on topic? [y/n]");
		scanf(" %c", &command);
	} while (command != 'n');
}

void print_received_message(SOCKET* socket, char* recvbuf, void* param) {
	int message_length = strlen(recvbuf + 2);
	if (message_length < 20) {
		printf("Message received from client: %s.\n", recvbuf + 2);
	}
	else {
		printf(" Received %d bytes\n", message_length);
	}
}

void print_subscribe_confirmation(SOCKET* socket, char* recvbuf, void* param) {
	if (recvbuf[2] == SUBSCRIBE_SUCCESS) {
		printf("SUCCESS!\n");
	}
	else {
		printf("FAIL!\n");
		closesocket(*socket);
	}
}

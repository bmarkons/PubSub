#include "stdafx.h"

void publishing_loop(SOCKET* socket) {
	char command;
	while (true) {
		printf("Do you want to publish message? [y/n]");
		scanf(" %c", &command);
		system("cls");

		if (command == 'n') {
			disconnect(socket);
			exit(EXIT_SUCCESS);
		}

		char message[MAX_INPUT_SIZE];
		char topic[MAX_INPUT_SIZE];
		input_message(message);
		input_topic(topic);
		system("cls");

		// make ByteArray structs
		ByteArray message_bytes;
		message_bytes.size = strlen(message);
		message_bytes.array = message;
		ByteArray topic_bytes;
		topic_bytes.size = strlen(topic);
		topic_bytes.array = topic;

		publish(message_bytes, topic_bytes, socket);
	}
}

void publish(ByteArray message, ByteArray topic, SOCKET* socket) {
	ByteArray package = make_package(message, topic);
	bool success = send_nonblocking(socket, package);
	if (success) {
		if (TEST == 0) { //show only when is not a test
			printf("Awesome! Message '%s' published on topic '%s'!\n", message.array, topic.array);
		}
	}
	else {
		printf("Error occured while publishing...\n");
	}
}

ByteArray make_package(ByteArray message, ByteArray topic) {
	ByteArray package;

	package.size = PACKAGE_HEADER_SIZE + topic.size + message.size;
	package.array = (char*)malloc(package.size * sizeof(char));

	u_short main_header = package.size - 2;	// size of the rest of package
	u_short topic_header = topic.size;		// size of topic
	u_short message_header = message.size;	// size of message
	memcpy(package.array, &main_header, sizeof(u_short));
	memcpy(package.array + 2, &topic_header, sizeof(u_short));
	memcpy(package.array + 4, &message_header, sizeof(short));

	memcpy(package.array + PACKAGE_HEADER_SIZE, topic.array, topic.size);
	memcpy(package.array + (PACKAGE_HEADER_SIZE + topic.size), message.array, message.size);

	return package;
}

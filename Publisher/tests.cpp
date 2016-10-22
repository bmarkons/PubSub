
#include "stdafx.h"

bool is_test() {

	char command;
	while (1) {

		printf("Start test? [y/n]\n");
		scanf(" %c", &command);
		system("cls");

		if (command == 'n' || command == 'N')
		{
			return false;
		}
		if (command == 'y' || command == 'Y') {
			return true;
		}
	}
}

void start_1GB_test(SOCKET *socket) {
	ByteArray message;
	message.size = 200;
	message.array = (char*)malloc(message.size * sizeof(char));
	memset(message.array, 'a', message.size);

	ByteArray topic;
	topic.size = 1;
	topic.array = (char*)malloc(topic.size * sizeof(char));
	topic.array[0] = '1';

	int num_of_msg = 1024 * 1024 * 5;

	for (int i = 0; i < num_of_msg; i++) {
		publish(message, topic, socket);
		//Sleep(100);
	}

}
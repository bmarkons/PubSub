
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
	char *message = (char*)calloc(202, sizeof(char));

	memset(message, 'a', 200);
	message[201] = 'b';

	char *topic = (char*)calloc(2, sizeof(char));
	topic[0] = 1 + '0';

	int num_of_msg = 1024 * 1024 * 5;

	for (int i = 0; i < num_of_msg; i++) {
		publish(message, topic, socket);
	}

}
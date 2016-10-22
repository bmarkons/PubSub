
#include "stdafx.h"

int main(int argc, char **argv)
{
	InitializeWindowsSockets();

	SOCKET connectSocket = INVALID_SOCKET;
	connectToServer(&connectSocket, argv[1], DEFAULT_PORT);
	printf("You are connected to PubSubEngine!\n");

	if (is_test()) {
		start_1GB_test(&connectSocket);
	}
	else {
		publishing_loop(&connectSocket);
	}
	getchar();
	disconnect(&connectSocket);
	return 0;
}


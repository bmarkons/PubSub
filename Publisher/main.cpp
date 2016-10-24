
#include "stdafx.h"


int main(int argc, char **argv)
{
	InitializeWindowsSockets();

	SOCKET connectSocket = INVALID_SOCKET;
	connectToServer(&connectSocket, argv[1], DEFAULT_PORT);
	printf("You are connected to PubSubEngine!\n");


	switch (TEST)
	{
	case 1:
		start_1GB_test(&connectSocket);
		break;
	case 2:
		start_test_2(&connectSocket);
		break;
	default:
		publishing_loop(&connectSocket);
	}

	getchar();
	disconnect(&connectSocket);
	return 0;
}


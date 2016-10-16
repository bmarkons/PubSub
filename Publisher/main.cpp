#include "stdafx.h"

int main(int argc, char **argv)
{
	InitializeWindowsSockets();

	SOCKET connectSocket = INVALID_SOCKET;
	connectToServer(&connectSocket, argv[1], DEFAULT_PORT);
	printf("You are connected to PubSubEngine!\n");

	publishing_loop(&connectSocket);

	disconnect(&connectSocket);
	return 0;
}
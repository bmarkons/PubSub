#include "stdafx.h"

int main(int argc, char **argv)
{
	InitializeWindowsSockets();

	SOCKET connectSocket = INVALID_SOCKET;
	connectToServer(&connectSocket, argv[1], DEFAULT_PORT);

	//test message
	//send(connectSocket, "marko", (int)strlen("srki gay") + 1, 0);

	disconnect(&connectSocket);
	return 0;
}
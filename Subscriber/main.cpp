// Subscriber.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define DEFAULT_BUFLEN 512

int main(int argc, char **argv)
{
	InitializeWindowsSockets();

	SOCKET connectSocket = INVALID_SOCKET;
	connectToServer(&connectSocket, argv[1], DEFAULT_PORT);

	printf("You are connected to PubSubEngine!\n");

	char topic;
	input_topic(&topic);

	subscribe(&connectSocket,topic);

	//waitForMessage(&connectSocket, DEFAULT_BUFLEN);
	getchar();
	disconnect(&connectSocket);
	return 0;
}


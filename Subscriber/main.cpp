// Subscriber.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define DEFAULT_BUFLEN 512  //1KB
#define IP_ADDRESS "127.0.0.1"

int main(int argc, char **argv)
{
	InitializeWindowsSockets();
	SOCKET connectSocket = INVALID_SOCKET;
	connectToServer(&connectSocket, IP_ADDRESS, DEFAULT_PORT);
	printf("You are connected to PubSubEngine!\n");
	if (argc > 2) {
		char* topic = argv[2];
		//subsribing for specific topic
		ByteArray topic_bytes;
		topic_bytes.size = strlen(topic);
		topic_bytes.array = topic;
		subscribe(&connectSocket, topic_bytes);

		//check if server returns a message that everything is ok
		checkConfimation(&connectSocket);
	}
	else {
		subscribing(&connectSocket);
	}
	wait_for_message(&connectSocket, DEFAULT_BUFLEN);

	disconnect(&connectSocket);
	return 0;
}


// PubSub.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void load_topics(List *topic_contents) {

	for (int i = 0; i < 10; i++) {
		TopicContent tc;
		tc.topic = i + '0';
		list_new(&tc.sockets, sizeof(SOCKET), free_socket);

		list_append(topic_contents, &tc);
	}
}

int main()
{
	InitializeWindowsSockets();

	DWORD accept_publisher_id, accept_subscriber_id;
	HANDLE accept_publisher_handle, accept_subscriber_handle;

	List topic_contents;
	list_new(&topic_contents, sizeof(TopicContent), free_topic_content);
	
	//Create some initial topics
	load_topics(&topic_contents);

	accept_publisher_handle = CreateThread(NULL, 0, &accept_publisher, &topic_contents, 0, &accept_publisher_id);
	accept_subscriber_handle = CreateThread(NULL, 0, &accept_subscriber, &topic_contents, 0, &accept_subscriber_id);

	getchar();

	list_destroy(&topic_contents);
	CloseHandle(accept_publisher_handle);
	CloseHandle(accept_subscriber_handle);
	
	// TODO: Close all handles and sockets

	return 0;
}
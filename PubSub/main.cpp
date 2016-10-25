// PubSub.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void load_topics(Wrapper *wrapper) {

	for (int i = 0; i < 10; i++) {
		ByteArray topic;
		topic.size = 1;
		topic.array = (char*)calloc(2, sizeof(char));
		topic.array[0] = i + '0';
		create_topic(wrapper, topic);
	}
}

int main()
{
	InitializeWindowsSockets();

	DWORD accept_publisher_id, accept_subscriber_id;
	DWORD thread_collector_id;
	HANDLE accept_publisher_handle, accept_subscriber_handle;
	HANDLE thread_collector_handle;

	List topic_contents;
	list_new(&topic_contents, sizeof(TopicContent), free_topic_content);

	List thread_list;
	list_new(&thread_list, sizeof(TThread), free_thread);

	SOCKET accepting_publisher = INVALID_SOCKET;
	SOCKET accepting_subscriber = INVALID_SOCKET;

	Wrapper wrapper;
	wrapper.thread_list = &thread_list;
	wrapper.topic_contents = &topic_contents;
	wrapper.accepting_publisher = &accepting_publisher;
	wrapper.accepting_subscriber = &accepting_subscriber;

	//Create some initial topics
	load_topics(&wrapper);

	accept_publisher_handle = CreateThread(NULL, 0, &accept_publisher, &wrapper, 0, &accept_publisher_id);
	accept_subscriber_handle = CreateThread(NULL, 0, &accept_subscriber, &wrapper, 0, &accept_subscriber_id);

	/*Add thread to list*/
	add_to_thread_list(wrapper.thread_list, accept_publisher_handle, accept_publisher_id);
	add_to_thread_list(wrapper.thread_list, accept_subscriber_handle, accept_subscriber_id);

	/*look for terminated thread and remove them*/
	thread_collector_handle = CreateThread(NULL, 0, &thread_collector, &wrapper, 0, &thread_collector_id);

	getchar();

	closesocket(accepting_publisher);
	closesocket(accepting_subscriber);

	TerminateThread(thread_collector_handle, 0);
	CloseHandle(thread_collector_handle);

	list_destroy(&thread_list);
	list_destroy(&topic_contents);

	WSACleanup();

	return 0;
}
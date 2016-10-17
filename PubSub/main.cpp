// PubSub.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
	DWORD accept_publisher_id;
	HANDLE accept_publisher_handle;

	InitializeWindowsSockets();

	accept_publisher_handle = CreateThread(NULL, 0, &accept_publisher, NULL, 0, &accept_publisher_id);
	//nit(accept_publisher);
	//nit(accept_subscriber);
	getchar();

	CloseHandle(accept_publisher_handle);

	return 0;
}
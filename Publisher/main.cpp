// Publisher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	SOCKET* connectSocket = (SOCKET*)calloc(1, sizeof(SOCKET));
	sockaddr_in* serverAddress = (sockaddr_in*)calloc(1, sizeof(sockaddr_in));

	if (InitializeWindowsSockets() == false) {
		return 1;
	}
	connectToServer(connectSocket, serverAddress);
	return 0;
}


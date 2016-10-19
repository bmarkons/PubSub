#include "stdafx.h"

void InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
}

void connectToServer(SOCKET* connectSocket, char* ipv4_address, u_int port)
{
	*connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*connectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ipv4_address);
	serverAddress.sin_port = htons(port);

	if (connect(*connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
		printf("Unable to connect to server. Error code: %d\n", WSAGetLastError());
		closesocket(*connectSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
}

void disconnect(SOCKET* socket) {
	closesocket(*socket);
	WSACleanup();
	printf("Connection closed.\n");
}

void publishing_loop(SOCKET* socket) {
	char command;
	while (true) {
		printf("Do you want to publish message? [y/n]");
		scanf(" %c", &command);
		system("cls");

		if (command == 'n') {
			disconnect(socket);
			exit(EXIT_SUCCESS);
		}

		char message, topic;
		input_message(&message);
		input_topic(&topic);
		system("cls");

		publish(message, topic, socket);
	}
}

void input_message(char* message) {
	printf("Input message (one character) :\n");
	scanf(" %c", message);
}

void input_topic(char* topic) {
	printf("Input topic (one character) :\n");
	scanf(" %c", topic);
}

void publish(char message, char topic, SOCKET* socket) {

	char* data_package = make_data_package(message, topic);
	int iResult = send(*socket, data_package, 3, 0);
	free(data_package);
	if (iResult == SOCKET_ERROR) {
		printf("Error occured while publishing...\n");
	}
	else {
		printf("Awesome! Message '%c' published on topic '%c'!\n", message, topic);
	}
}

char* make_data_package(char message, char topic) {
	int size_of_package = 2; 
	char* data_package = (char*)malloc(sizeof(char)*(size_of_package + 1));
	data_package[0] = size_of_package;
	data_package[1] = topic;
	data_package[2] = message;

	return data_package;
}
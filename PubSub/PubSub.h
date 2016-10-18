#pragma once

#define SERVER_SLEEP_TIME 50
#define DEFAULT_BUFLEN 512

void InitializeWindowsSockets();
DWORD WINAPI accept_publisher(LPVOID);
DWORD WINAPI listen_publisher(LPVOID lpParam);
void start_listening(SOCKET* listenSocket, char* port);
void waitForMessage(SOCKET*, unsigned);
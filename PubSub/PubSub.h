#pragma once

void InitializeWindowsSockets();
DWORD WINAPI accept_publisher(LPVOID);
DWORD WINAPI listen_publisher(LPVOID lpParam);
void start_listening(SOCKET* listenSocket, char* port);
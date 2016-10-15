#pragma once

void InitializeWindowsSockets();
void connectToServer(SOCKET*, char*, u_int);
void disconnect(SOCKET*);
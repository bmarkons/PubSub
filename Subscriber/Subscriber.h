#pragma once

void subscribe(SOCKET* socket, ByteArray topic);
ByteArray make_package(ByteArray topic);
void checkConfimation(SOCKET* socket);
void subscribing(SOCKET*);
void print_received_message(SOCKET* socket, char* recvbuf, void* param);
#pragma once

void InitializeWindowsSockets();
void connectToServer(SOCKET* socket, char* ipv4_address, u_int port);
void disconnect(SOCKET*);
void publishing_loop(SOCKET* socket);
void input_message(char* message);
void input_topic(char* topic);
void subscribe(SOCKET* socket, char topic);
void make_data_package(char message, char topic, char* data_package);
void waitForMessage(SOCKET* socket,unsigned buffer_size);

#include "stdafx.h"
#using <System.dll>
#include <direct.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::ComponentModel;

#define ABSOLUTE_PATH "C:\\Users\\Srdjan\\Documents\\GitHub\\PubSub\\x64\\Debug\\"

bool is_test() {

	char command;
	while (1) {

		printf("Start test? [y/n]\n");
		scanf(" %c", &command);
		system("cls");

		if (command == 'n' || command == 'N')
		{
			return false;
		}
		if (command == 'y' || command == 'Y') {
			return true;
		}
	}
}

void start_1GB_test(SOCKET *socket) {
	ByteArray message;
	message.size = 1024;
	message.array = (char*)malloc(message.size * sizeof(char));
	memset(message.array, 'a', message.size);

	ByteArray topic;
	topic.size = 1;
	topic.array = (char*)malloc(topic.size * sizeof(char));
	topic.array[0] = '1';

	int num_of_msg = 1024 * 1024;
	float percent = 0;
	for (int i = 0; i < num_of_msg; i++) {

		percent = ((float)i / (float)num_of_msg) * 100;
		system("cls");
		printf("---TEST 1-1GB---\n");
		printf(" - TO COMPLETE : %0.2f %%", percent);
		publish(message, topic, socket);
		Sleep(100);
	}

}

void start_test_2(SOCKET *socket) {

	for (int i = 0; i < 10; i++) {
		Process::Start(String::Concat(ABSOLUTE_PATH, "Subscriber.exe"), String::Concat("127.0.0.1 ", System::Convert::ToString(i)));
		Sleep(100);
	}


	ByteArray message;
	message.size = 100;
	message.array = (char*)malloc(message.size * sizeof(char));
	

	ByteArray topic;

	for (int i = 0; i < 100; i++) {
		system("cls");
		printf("---TEST 2---\n");
		printf(" - TO COMPLETE : %d %%", i);

		for (int j = 0; j < 10; j++) {
			memset(message.array, j + 'a', message.size);
			topic.size = 1;
			topic.array = (char*)malloc(topic.size * sizeof(char));
			topic.array[0] = j + '0';
			publish(message, topic, socket);
			Sleep(50);
		}
	}

}
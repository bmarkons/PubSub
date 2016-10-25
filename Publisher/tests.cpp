
#include "stdafx.h"
#using <System.dll>
#include <direct.h>
#include <time.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::ComponentModel;

#define ABSOLUTE_PATH "C:\\Users\\Srdjan\\Documents\\GitHub\\PubSub\\x64\\Debug\\"
//#define ABSOLUTE_PATH "D:\\Fakultet\\BLOK 1\\PubSub\\x64\\Debug\\"

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

	clock_t begin = clock();
	int min, sec;
	float total_sec;
	float speed;

	for (int i = 0; i < num_of_msg; i++) {
		total_sec = (float)(clock() - begin) / CLOCKS_PER_SEC;
		speed = (float)(i + 1) / total_sec;

		min = total_sec / 60;
		sec = (int)total_sec % 60;

		percent = ((float)(i + 1) / (float)num_of_msg) * 100;
		system("cls");
		printf("---TEST 1-1GB---\n");
		printf(" - TO COMPLETE : %0.2f %%\n", percent);
		printf(" - ELAPSED TIME: %d:%d\n", min, sec);
		printf(" - SPEED: %0.2f KB/s\n", speed);
		printf(" - SENT: %0.2f MB\n", (float)(i + 1) / 1024);


		publish(message, topic, socket);
	}

}

void start_test_2(SOCKET *socket) {

	for (int i = 0; i < 10; i++) {
		Process::Start(String::Concat(ABSOLUTE_PATH, "Subscriber.exe"), String::Concat("127.0.0.1 ", System::Convert::ToString(i)));
		Sleep(50);
	}


	ByteArray message;
	message.size = 1000;
	message.array = (char*)malloc(message.size * sizeof(char));
	

	ByteArray topic;
	int num_of_msg = 100;
	int percent;
	for (int i = 0; i < num_of_msg; i++) {
		percent = ((float)(i + 1) / (float)num_of_msg) * 100;

		system("cls");
		printf("---TEST 2---\n");
		printf(" - TO COMPLETE : %d %%", percent);

		for (int j = 0; j < 10; j++) {
			memset(message.array, j + 'a', message.size);
			topic.size = 1;
			topic.array = (char*)malloc(topic.size * sizeof(char));
			topic.array[0] = j + '0';
			publish(message, topic, socket);
		}
	}

}
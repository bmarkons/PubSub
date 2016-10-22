#pragma once

#define WIN32_LEAN_AND_MEAN
#define LISTEN_PUBLISHER_PORT "27017"
#define LISTEN_SUBSCRIBER_PORT "27018"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>

typedef struct _byte_array {
	u_short size;
	char* array;
}ByteArray;

#include "list.h"
#include "buffer.h"
#include "PubSub.h"


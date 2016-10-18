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

#include "list.h"
#include "PubSub.h"


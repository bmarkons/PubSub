//#include "stdafx.h"

#define MIN_BUFFER_SIZE 10
#define MAX_BUFFER_SIZE 10240
#define TYPE ByteArray

typedef struct _tbuffer{
	TYPE *buffer;
	int pushldx;
	int popldx;
	int count;
	int bufferSize;
	CRITICAL_SECTION cs;
}TBuffer;

bool Push(TBuffer *buffer, TYPE number);
bool Pop(TBuffer *buffer, TYPE *retVal);
void InitializeBuffer(TBuffer *buffer, int bufferSize);
void PrintBuffer(TBuffer *buffer);
void ExpandBuffer(TBuffer* oldBuffer);
void NarrowBuffer(TBuffer* oldBuffer);
void DestoyBuffer(TBuffer *buffer);
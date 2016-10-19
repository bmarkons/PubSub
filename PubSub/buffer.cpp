#pragma once

#include "stdafx.h"

/*THread safe push for circ*/
bool Push(TBuffer *buffer, TYPE data) {

	EnterCriticalSection(&buffer->cs);

	if (buffer->count == buffer->bufferSize) {
		if (buffer->bufferSize >= MAX_BUFFER_SIZE) {
			LeaveCriticalSection(&buffer->cs);
			return false;
		}
		printf("\nExpanding buffer!");
		ExpandBuffer(buffer);
	}

	buffer->buffer[buffer->pushldx] = data;
	buffer->pushldx++;
	if (buffer->pushldx == buffer->bufferSize) {
		buffer->pushldx = 0;
	}
	buffer->count++;

	LeaveCriticalSection(&buffer->cs);
	return true;
}

bool Pop(TBuffer *buffer, TYPE *retVal) {

	EnterCriticalSection(&buffer->cs);

	if (buffer->count == 0) {
		LeaveCriticalSection(&buffer->cs);
		return false;
	}

	int usageInPercent = ((float)buffer->count / (float)buffer->bufferSize) * 100;
	if (usageInPercent < 25 && buffer->bufferSize > MIN_BUFFER_SIZE) {
		//when buffer is under 25% full and bufferSize bigger than minimal value of bufferSize
		printf("\nBuffer is %d%% full,buffer size: %d", usageInPercent, buffer->bufferSize);
		printf("\nBuffer is narrowing..");
		NarrowBuffer(buffer);
	}

	*retVal = buffer->buffer[buffer->popldx];
	buffer->popldx++;
	if (buffer->popldx == buffer->bufferSize) {
		buffer->popldx = 0;
	}

	buffer->count--;

	LeaveCriticalSection(&buffer->cs);
	return true;
}

void PrintBuffer(TBuffer *buffer) {

	EnterCriticalSection(&buffer->cs);
	printf("\n\n***PRINT BUFFER***\n");
	printf(" -Count: %d\n", buffer->count);
	printf(" -PopIndex: %d\n", buffer->popldx);
	printf(" -PushIndex: %d\n", buffer->pushldx);
	printf(" -Buffer SIZE: %d\n", buffer->bufferSize);
	printf("[");
	for (int i = 0; i < buffer->bufferSize; i++) {
		if (i % 10 == 0) {
			printf("\n");
		}
		printf(" %c ", buffer->buffer[i]);
	}
	printf("]\n");
	LeaveCriticalSection(&buffer->cs);

}

void ExpandBuffer(TBuffer * circBuffer)
{
	int newBufferSize = circBuffer->bufferSize * 2;
	char *newBuffer = (char*)calloc(newBufferSize, sizeof(TYPE));
	//InitializeBuffer(&retBuffer, oldBuffer->bufferSize * 2);//prosiruje se duplo

	int deltaMem = circBuffer->bufferSize - circBuffer->pushldx; // razlika izmedju push i buffer size

	memcpy(newBuffer,									//dest
		circBuffer->buffer + circBuffer->pushldx,				//src
		deltaMem * sizeof(TYPE));								//length
	memcpy(newBuffer + deltaMem,						//dest
		circBuffer->buffer,									//src
		(circBuffer->bufferSize - deltaMem) * sizeof(TYPE));		// length

	free(circBuffer->buffer);

	circBuffer->popldx = 0;
	circBuffer->pushldx = circBuffer->count;
	circBuffer->buffer = newBuffer;
	circBuffer->bufferSize = newBufferSize;

}

void NarrowBuffer(TBuffer * circBuffer)
{
	int newBufferSize = circBuffer->bufferSize / 2;
	TYPE *newBuffer = (TYPE*)calloc(newBufferSize, sizeof(TYPE));


	if (circBuffer->popldx <= circBuffer->pushldx) {
		//just move data beetwen popIdx and pushIdx to newBuffer
		int deltaMem = circBuffer->pushldx - circBuffer->popldx;
		memcpy(newBuffer,												//dest
			circBuffer->buffer + circBuffer->popldx,				//src
			deltaMem * sizeof(TYPE));								//length
	}
	else {
		//first move data beetwen popIdx and bufferSize to newBuffer
		//than data beetwen 0 and pushIdx from oldBuffer to newBuffer
		int deltaMem = circBuffer->bufferSize - circBuffer->popldx;
		memcpy(newBuffer,												//dest
			circBuffer->buffer + circBuffer->popldx,				//src
			deltaMem * sizeof(TYPE));								//length

		memcpy(newBuffer + deltaMem,									//dest
			circBuffer->buffer,										//src
			circBuffer->pushldx * sizeof(TYPE));						//length beetwen 0 and pushIdx
	}

	circBuffer->popldx = 0;
	circBuffer->pushldx = circBuffer->count;
	circBuffer->buffer = newBuffer;
	circBuffer->bufferSize = newBufferSize;
}

void DestoyBuffer(TBuffer * buffer)
{
	free(buffer->buffer);
	DeleteCriticalSection(&buffer->cs);
}

void InitializeBuffer(TBuffer *buffer, int bufferSize) {
	InitializeCriticalSection(&buffer->cs);
	buffer->bufferSize = bufferSize;
	buffer->buffer = (TYPE*)malloc(sizeof(TYPE)*bufferSize);
	buffer->count = 0;
	buffer->popldx = 0;
	buffer->pushldx = 0;
}
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

/**
	Push new message in a given circular buffer

	@param buffer
	@param message
	@return success feedback
*/
bool Push(TBuffer *buffer, TYPE message);
/**
	Pop new message from given circular buffer

	@param buffer
	@param retVal - pop message
	@return success feedback
*/
bool Pop(TBuffer *buffer, TYPE *retVal);
/**
	Initialize new buffer with init size

	@param buffer
	@param bufferSize
*/
void InitializeBuffer(TBuffer *buffer, int bufferSize);
/**
	Print buffer with all its messages

	@param buffer
*/
void PrintBuffer(TBuffer *buffer);
/**
	Expand given buffer by double

	@param oldBuffer
*/
void ExpandBuffer(TBuffer* oldBuffer);
/**
	Narrow given buffer by double

	@param oldBuffer
*/
void NarrowBuffer(TBuffer* oldBuffer);
/**
	Destroy given buffer

	@param buffer
*/
void DestoyBuffer(TBuffer *buffer);
/*
 * ring_buffer.h
 *
 *  Created on: 7 lip 2015
 *      Author: Rafa³ Mszal
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct{
	uint8_t *buffer;
	uint8_t *head;
	uint8_t *tail;
	size_t buffSize;
	size_t elementsCnt;
} RingBuffer_t;


bool RingBuffer_Init(RingBuffer_t *ringBuffer, uint8_t *buffer, size_t bufferSize);
bool RingBuffer_PutChar(RingBuffer_t *ringBuffer, char ch);
bool RingBuffer_GetChar(RingBuffer_t *ringBuffer, char *ch);
bool RingBuffer_IsEmpty(RingBuffer_t *ringBuffer);
void RingBuffer_Clear(RingBuffer_t *ringBuffer);
size_t RingBuffer_GetLen(RingBuffer_t *ringBuffer);
size_t RingBuffer_GetCapacity(RingBuffer_t *ringBuffer);

#endif

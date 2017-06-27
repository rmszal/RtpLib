/*
 * ring_buffer.c
 *
 *  Created on: 7 lip 2015
 *      Author: Rafa³ Mszal
 */

#include "ring_buffer.h"
#include <assert.h>
#include <stdio.h>
#include "stm32f7xx.h"

bool RingBuffer_Init(RingBuffer_t *ringBuffer, uint8_t *buffer, size_t bufferSize)
{
	assert(ringBuffer);
	assert(buffer);
	assert(bufferSize > 0);

	if(ringBuffer && buffer && bufferSize > 0)
	{
		ringBuffer->buffer = buffer;
		ringBuffer->head = buffer;
		ringBuffer->tail = buffer;
		ringBuffer->buffSize = bufferSize;
		ringBuffer->elementsCnt = 0;
		return true;
	}
	return false;
}

bool RingBuffer_PutChar(RingBuffer_t *ringBuffer, char ch)
{
	assert(ringBuffer);

	__disable_irq();

	if(ringBuffer->elementsCnt < ringBuffer->buffSize)
	{
		*ringBuffer->head = ch;
		ringBuffer->elementsCnt++;
		if(ringBuffer->head >= ringBuffer->buffer + ringBuffer->buffSize)
		{
			ringBuffer->head = ringBuffer->buffer;
		}
		else
		{
			ringBuffer->head++;
		}

		__enable_irq();
		return true;
	}

	__enable_irq();
	return false;
}

bool RingBuffer_GetChar(RingBuffer_t *ringBuffer, char *ch)
{
	assert(ringBuffer);
	assert(ch);
	if(RingBuffer_IsEmpty(ringBuffer))
	{
		*ch = 0;
		return false;
	}
	else
	{
		__disable_irq();

		*ch = *ringBuffer->tail;
		ringBuffer->elementsCnt--;
		if(ringBuffer->tail >= ringBuffer->buffer + ringBuffer->buffSize)
		{
			ringBuffer->tail = ringBuffer->buffer;
		}
		else
		{
			ringBuffer->tail++;
		}

		__enable_irq();

		return true;
	}
}

bool RingBuffer_IsEmpty(RingBuffer_t *ringBuffer)
{
	assert(ringBuffer);
	return RingBuffer_GetLen(ringBuffer) == 0;
}

void RingBuffer_Clear(RingBuffer_t *ringBuffer)
{
	assert(ringBuffer);
	ringBuffer->head = ringBuffer->buffer;
	ringBuffer->tail = ringBuffer->buffer;
	ringBuffer->elementsCnt = 0;
}

size_t RingBuffer_GetLen(RingBuffer_t *ringBuffer)
{
	assert(ringBuffer);
	return ringBuffer->elementsCnt;
}

size_t RingBuffer_GetCapacity(RingBuffer_t *ringBuffer)
{
	assert(ringBuffer);
	return ringBuffer->buffSize;
}


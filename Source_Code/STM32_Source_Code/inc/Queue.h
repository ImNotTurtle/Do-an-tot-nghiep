/*
 * Queue.h
 *
 *  Created on: Mar 14, 2024
 *      Author: Administrator
 */

#ifndef QUEUE_H_
#define QUEUE_H_
#include <stdint.h>
#include <stdbool.h>

typedef struct{
	uint8_t *buffer;
	uint8_t bufferSize;
	uint8_t frontPos;
	uint8_t backPos;
}queue_t, *queue_p;

queue_p Queue_Init(uint8_t bufferSize);
bool Queue_IsEmpty(queue_p queue);
bool Queue_IsFull(queue_p queue);
void Queue_EnQueue(queue_p queue, uint8_t data);
bool Queue_DeQueue(queue_p queue, uint8_t *data);
bool Queue_GetFront(queue_p queue, uint8_t *data);


#endif /* QUEUE_H_ */

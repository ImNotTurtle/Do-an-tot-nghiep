/*
 * Receive.h
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */

#ifndef SOURCE_APP_RECEIVE_RECEIVE_H_
#define SOURCE_APP_RECEIVE_RECEIVE_H_

#include "app/framework/include/af.h"

#include <stdbool.h>

typedef enum{
	DEVICE_CONNECTED,
	LEVEL_CONTROL,
}RECEIVE_CMD_e;
typedef void (*receiveHandle)(RECEIVE_CMD_e cmd, uint8_t length, uint8_t *payload);

void Receive_Init(receiveHandle handler);
boolean emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* im);
bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd);

#endif /* SOURCE_APP_RECEIVE_RECEIVE_H_ */

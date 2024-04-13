/*
 * Receive.h
 *
 *  Created on: Feb 28, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_APP_RECEIVE_RECEIVE_H_
#define SOURCE_APP_RECEIVE_RECEIVE_H_
#include "app/framework/include/af.h"
#include <stdbool.h>

#include "Source/Utils/DeviceManager/DeviceManager.h"

typedef enum{
	DEVICE_JOIN_NETWORK,
	DEVICE_CONNECTED,
	DEVICE_DISCONNECTED,
	LEVEL_CONTROL,
	DEVICE_LEAVE_NETWORK
}RECEIVE_CMD_ID_e;


typedef void (*receiveHandler)(EmberNodeId nodeID, RECEIVE_CMD_ID_e receiveId, uint8_t payloadLength, uint8_t *payload);

void Receive_Init(receiveHandler cb);
bool emberAfPreZDOMessageReceivedCallback(EmberNodeId emberNodeId,
                                          EmberApsFrame* apsFrame,
                                          int8u* message,
                                          int16u length);
bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd);



#endif /* SOURCE_APP_RECEIVE_RECEIVE_H_ */

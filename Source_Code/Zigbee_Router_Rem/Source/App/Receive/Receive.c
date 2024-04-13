/*
 * Receive.c
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */
#include "Receive.h"

static receiveHandle s_receiveHandler = NULL;

void Receive_Init(receiveHandle handler){
	s_receiveHandler = handler;
}

boolean emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* im){
	return false;
}

bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd){
	uint16_t attrID = (uint16_t)(cmd->buffer[cmd->payloadStartIndex + 1] << 8 | cmd->buffer[cmd->payloadStartIndex]);
	uint16_t bufIndex = cmd->payloadStartIndex + 4; //buffer payload start index
	switch(cmd->apsFrame->clusterId){
	case ZCL_LEVEL_CONTROL_CLUSTER_ID:
	{
		if(attrID == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID){
			//receive level control from HC
			const uint8_t payloadLength = 1;
			uint8_t payload[payloadLength];
			payload[0] = cmd->buffer[bufIndex];
			if(s_receiveHandler != NULL){
				s_receiveHandler(LEVEL_CONTROL, payloadLength, payload);
			}
		}
		return true;
	}
	default:
		break;
	}
	return false;
}

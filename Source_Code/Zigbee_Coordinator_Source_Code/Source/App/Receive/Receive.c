/*
 * Receive.c
 *
 *  Created on: Feb 28, 2024
 *      Author: Administrator
 */

#include "Receive.h"

static receiveHandler s_receiveHandler = NULL;

void Receive_Init(receiveHandler cb){
	s_receiveHandler = cb;
}

bool emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incomingMessage){
	return false;
}

bool emberAfPreZDOMessageReceivedCallback(EmberNodeId emberNodeId,
                                          EmberApsFrame* apsFrame,
                                          int8u* message,
                                          int16u length){
	if(apsFrame->profileId == EMBER_ZDO_PROFILE_ID){
		switch(apsFrame->clusterId){
		case LEAVE_RESPONSE:
		{
			if(s_receiveHandler){
				s_receiveHandler(emberNodeId, DEVICE_LEAVE_NETWORK, 0, NULL);
			}
			return true;
		}
		default:
			break;
		}
	}

	return false;
}

bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd){
	EmberNodeId nodeId = cmd->source;
	uint16_t attrID = (uint16_t)(cmd->buffer[cmd->payloadStartIndex + 1] << 8 | cmd->buffer[cmd->payloadStartIndex]);
	uint16_t bufIndex = cmd->payloadStartIndex + 4; //buffer payload start index
	switch(cmd->apsFrame->clusterId){
	case ZCL_BASIC_CLUSTER_ID:
	{
		//if receive a model id from a device
		if(attrID == ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID){
			//get the model id from payload
			uint8_t modelID[30] = {0};
			uint8_t index = 0;
			for(; bufIndex < cmd->bufLen; index++, bufIndex++){
				modelID[index] = cmd->buffer[bufIndex];
			}
			if(s_receiveHandler != NULL){
				s_receiveHandler(nodeId, DEVICE_JOIN_NETWORK, index, modelID);
			}
		}
		return true;
	}
	case ZCL_LEVEL_CONTROL_CLUSTER_ID:
	{
		if(attrID == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID && cmd->commandId == ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID){
			//receive level control from device, send to the server
			uint8_t payload[1];
			payload[0] = cmd->buffer[bufIndex];
			if(s_receiveHandler != NULL){
				s_receiveHandler(nodeId, LEVEL_CONTROL, 1, payload);
			}
		}
		return true;
	}

	default:
		break;
	}


	return false;
}

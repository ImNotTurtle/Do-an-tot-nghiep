/*
 * Send.c
 *
 *  Created on: Mar 2, 2024
 *      Author: Administrator
 */
#include "Send.h"

#define MAX_DATA_COMMAND_SIZE				50


static void FillBufferGlobalCommand(EmberAfClusterId clusterId,
							 	  EmberAfAttributeId attributeId,
								  uint8_t commandId,
								  uint8_t *pDataRead,
								  uint8_t length,
								  uint8_t dataType);




void Send_SendCommandUnicast(uint8_t srcEP, uint8_t destEP, EmberNodeId nodeId){
	emberAfSetCommandEndpoints(srcEP, destEP);
	(void)emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
}


void Send_LevelControl(EmberNodeId nodeId, uint8_t srcEp, uint8_t destEp, uint8_t value){
	uint8_t payload[1];
	payload[0] = value;
	FillBufferGlobalCommand(ZCL_LEVEL_CONTROL_CLUSTER_ID,
							ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
							ZCL_WRITE_ATTRIBUTES_COMMAND_ID,
							payload,
							1,
							ZCL_INT8U_ATTRIBUTE_TYPE);
	Send_SendCommandUnicast(srcEp, destEp, nodeId);
}


static void FillBufferGlobalCommand(EmberAfClusterId clusterId,
							 	  EmberAfAttributeId attributeId,
								  uint8_t commandId,
								  uint8_t *pDataRead,
								  uint8_t length,
								  uint8_t dataType){
	uint8_t data[MAX_DATA_COMMAND_SIZE];
	data[0] = (uint8_t)(attributeId & 0x00FF);
	data[1] = (uint8_t)((attributeId & 0xFF00) >> 8);
	data[2] = EMBER_SUCCESS;
	data[3] = dataType;
	memcpy(&data[4], pDataRead, length);

	(void)emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND |
									 ZCL_FRAME_CONTROL_CLIENT_TO_SERVER |
									 ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
									clusterId, commandId, "b", data, length + 4);
}

void Send_SendViaBinding(uint8_t localEndpoint, uint8_t remoteEndpoint, bool value, uint16_t nodeID){
	EmberStatus status = EMBER_INVALID_BINDING_INDEX;
	for(uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++){
		EmberBindingTableEntry binding;
		status = emberGetBinding(i, &binding);
		uint16_t bindingNodeID = emberGetBindingRemoteNodeId(i);
		if(status != EMBER_SUCCESS){
			return;
		}else if((bindingNodeID == nodeID) && (binding.local == localEndpoint) && (binding.remote == remoteEndpoint)){
			//ignore source node, duplicated
			continue;
		}
		else if(bindingNodeID == emberAfGetNodeId()){
			//ignore self node, self binding
			continue;
		}
		else if((bindingNodeID != EMBER_SLEEPY_BROADCAST_ADDRESS) &&
				(bindingNodeID != EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS) &&
				 (bindingNodeID != EMBER_BROADCAST_ADDRESS)){
			if(binding.local == localEndpoint){
				switch(value){
				case true:
					emberAfFillCommandOnOffClusterOn();
					emberAfSetCommandEndpoints(binding.local, binding.remote);
					emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);
					break;
				case false:
					emberAfFillCommandOnOffClusterOff();
					emberAfSetCommandEndpoints(binding.local, binding.remote);
					emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);
				}
			}
		}
	}
}

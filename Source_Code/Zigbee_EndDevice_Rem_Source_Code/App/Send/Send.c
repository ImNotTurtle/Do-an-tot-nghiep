/*
 * Send.c
 *
 *  Created on: Sep 3, 2023
 *      Author: Administrator
 */
#include "app/framework/include/af.h"
#include "Send.h"

#define MAX_DATA_COMMAND_SIZE				50

static void SEND_FillBufferGlobalCommand(EmberAfClusterId clusterId,
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

void SEND_SendCommandUnicast(uint8_t srcEP, uint8_t destEP, uint32_t nwkAddress){
	emberAfSetCommandEndpoints(srcEP, destEP);
	(void)emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nwkAddress);
}
void SEND_ZigDevRequest(){
	uint8_t contents[2];
	contents[1] = 0x00;
	emberSendZigDevRequest(HC_NETWORK_ADDRESS, LEAVE_RESPONSE, EMBER_AF_DEFAULT_APS_OPTIONS, contents, sizeof(contents));
}

void SEND_ReportInfoToHC(){
	uint8_t modelID[] = {'R','E','M'};

	if(emberAfNetworkState() != EMBER_JOINED_NETWORK){
		return;
	}
	//send model to HC
	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 modelID,
								 sizeof(modelID),
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE
	);
	SEND_SendCommandUnicast(SOURCE_ENDPOINT_PRIMARY,
							DESTINATION_ENDPOINT,
							HC_NETWORK_ADDRESS);
}



void SEND_LevelStateReport(uint8_t srcEP, uint8_t value){
	uint8_t level = value;
	SEND_FillBufferGlobalCommand(ZCL_LEVEL_CONTROL_CLUSTER_ID,
								 ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t*)&value,
								 1,
								 ZCL_INT8U_ATTRIBUTE_TYPE);

	SEND_SendCommandUnicast(srcEP,
							DESTINATION_ENDPOINT,
							HC_NETWORK_ADDRESS);

	emberAfWriteServerAttribute(srcEP,
								ZCL_LEVEL_CONTROL_CLUSTER_ID,
								ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
								(uint8_t*)&level,
								ZCL_INT8U_ATTRIBUTE_TYPE);
}


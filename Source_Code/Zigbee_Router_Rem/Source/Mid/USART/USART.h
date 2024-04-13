/*
 * USART.h
 *
 *  Created on: Feb 20, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_DRIVER_USART_USART_H_
#define SOURCE_DRIVER_USART_USART_H_
#include "app/framework/include/af.h"
#include "Source/Utils/Queue/Queue.h"
#include <stdbool.h>
#include <stdlib.h>

#define NODE_ID_NOT_CARE							0xFFFE
#define ENDPOINT_NOT_CARE							0xFE

#define USART1_QUEUE_BUFFER_SIZE					128
#define USART1_SCAN_INTERVAL						1
#define USART1_SCAN_LATE_DELAY_INTERVAL				100
#define USART_PROCESS_SERIAL_INTERVAL				200

#define USART_FRAME_START_OF_FRAME					0xB1
#define USART_FRAME_PAYLOAD_MAX_LENGTH				4
#define USART_FRAME_CHECKXOR_INIT_VALUE				0xFF

typedef enum{
	BUFFER_ERROR,
	BUFFER_FULL,
	BUFFER_NOT_FULL,
	BUFFER_EMPTY,
	BUFFER_NOT_EMPTY
}USART_BUFFER_STATE;



typedef struct{
	uint8_t length;
	EmberNodeId nodeId;
	uint8_t endpoint;
	uint8_t id;
	uint8_t type;
	uint8_t payloadLength;
	uint8_t payload[USART_FRAME_PAYLOAD_MAX_LENGTH];
	uint8_t sequence;
	uint8_t cxor;
}USART_FRAME;


//USART_FRAME_IDs
#define USART_FRAME_ID_DEVICE_CONNECTED			0x07
#define USART_FRAME_ID_LEVEL_CONTROL			0x09

//USART_FRAME_TYPEs
#define USART_FRAME_TYPE_SET					0x00
#define USART_FRAME_TYPE_GET					0x01
#define USART_FRAME_TYPE_RESPONSE				0x02

#define USART_FRAME_DEFAULT					{0x00, 0x0000, 0x00, 0x00, 0x00, 0x00, {0x00}, 0x00, 0x00}

typedef void (*usartFrameCallback)(USART_FRAME);




void USART1_Init(usartFrameCallback cb);
void USART_SendFrame(uint8_t port, USART_FRAME frame);
USART_FRAME USART_GenerateFrame(uint16_t nodeId,
						 uint8_t endpoint,
						 uint8_t cmdId,
						 uint8_t cmdType,
						 uint8_t payloadLength,
						 uint8_t *payload);
USART_FRAME USART_GenerateDeviceConnected(void);
USART_FRAME USART_GenerateLevelControl(uint8_t levelCmd, uint8_t level);


#endif /* SOURCE_DRIVER_USART_USART_H_ */

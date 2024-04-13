/*
 * USART.h
 *
 *  Created on: Mar 14, 2024
 *      Author: Administrator
 */

#ifndef USART_H_
#define USART_H_
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "stm32f401re.h"
#include "stm32f401re_gpio.h"
#include "stm32f401re_usart.h"
#include "stm32f401re_rcc.h"
#include "misc.h"
#include "Queue.h"
#include "timer.h"

#define USART_FRAME_PAYLOAD_MAX_LENGTH				4
#define USART_PROCESS_SERIAL_INTERVAL				200

#define NODE_ID_NOT_CARE							0xFFFE
#define ENDPOINT_NOT_CARE							0xFE


typedef enum{
	BUFFER_ERROR,
	BUFFER_FULL,
	BUFFER_NOT_FULL,
	BUFFER_EMPTY,
	BUFFER_NOT_EMPTY
}USART_BUFFER_STATE;

typedef enum{
	LEVEL_SET,
	LEVEL_INCREASE,
	LEVEL_DECREASE,
	LEVEL_NONE = 0xFF
}LEVEL_CONTROL_CMD_e;


typedef struct{
	uint8_t length;
	uint16_t nodeId;
	uint8_t endpoint;
	uint8_t id;
	uint8_t type;
	uint8_t payloadLength;
	uint8_t payload[USART_FRAME_PAYLOAD_MAX_LENGTH];
	uint8_t sequence;
	uint8_t cxor;
}USART_FRAME;

/*	PRIVATE MACRO----------------------------------------------------------------*/
#define USART6_GPIO_PORT						GPIOC
#define USART6_TX_GPIO_PIN						GPIO_Pin_6
#define USART6_GPIO_RCC							RCC_AHB1Periph_GPIOC
#define USART6_CLOCK							RCC_APB2Periph_USART6

#define USART1_GPIO_PORT						GPIOB
#define USART1_RX_GPIO_PIN						GPIO_Pin_7
#define USART1_GPIO_RCC							RCC_AHB1Periph_GPIOB
#define USART1_CLOCK							RCC_APB2Periph_USART1

#define DATA1_CHECK								0x10
#define DATA2_CHECK								0x20
#define USART_BAUD								115200

#define USART_FRAME_START_OF_FRAME					0xB1

//USART_FRAME_IDs
#define USART_FRAME_CMD_ID_DEVICE_REPORT			0x04
#define USART_FRAME_CMD_ID_DEVICE_CONNECTED			0x07
#define USART_FRAME_CMD_ID_DEVICE_DISCONNECTED		0x08
#define USART_FRAME_CMD_ID_LEVEL_CONTROL			0x09

//USART_FRAME_TYPEs
#define USART_FRAME_CMD_TYPE_SET					0x00
#define USART_FRAME_CMD_TYPE_GET					0x01
#define USART_FRAME_CMD_TYPE_RESPONSE				0x02

//Level-Control frame payload
#define LEVEL_CONTROL_PAYLOAD_CMD_SET				0x00
#define LEVEL_CONTROL_PAYLOAD_CMD_INC				0x01
#define LEVEL_CONTROL_PAYLOAD_CMD_DEC				0x02

#define USART_FRAME_CHECKXOR_INIT_VALUE				0xFF

#define USART_FRAME_DEFAULT					{0x00, 0x0000, 0x00, 0x00, 0x00, 0x00, {0x00}, 0x00, 0x00}


typedef void (*usartFrameCallback)(USART_FRAME);

void MyUSART_Init(usartFrameCallback handler);
void USART_SendFrame(USART_FRAME frame);
USART_FRAME USART_GenerateFrame(uint16_t nodeId,
						 uint8_t endpoint,
						 uint8_t cmdId,
						 uint8_t cmdType,
						 uint8_t payloadLength,
						 uint8_t *payload);
USART_FRAME USART_GenerateDeviceConnected(void);
USART_FRAME USART_GenerateDeviceDisconnected(void);
USART_FRAME USART_GenerateLevelControl(LEVEL_CONTROL_CMD_e levelCmd, uint8_t level);
USART_FRAME USART_GenerateDeviceReport(void);




#endif /* USART_H_ */

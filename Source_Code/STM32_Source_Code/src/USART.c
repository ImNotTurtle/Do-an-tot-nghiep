/*
 * USART.c
 *
 *  Created on: Mar 14, 2024
 *      Author: Administrator
 */
#include "USART.h"

static usartFrameCallback usartRxHandler = NULL;
static queue_p usartRxQueue;
static SSwTimer s_idProcessSerialTimer = NO_TIMER;

static void USART_SendByte(uint8_t byte);
static uint8_t CalcXor(USART_FRAME frame);
static bool CheckXor(USART_FRAME frame);
static USART_FRAME PollRxBuffer(USART_TypeDef* USARTx);
static void USART6_Init(void);
static void USART1_Init(usartFrameCallback handler);
static void ProcessSerialReceiver(void);


void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE)==SET){
		uint8_t data = (uint8_t)(USART_ReceiveData(USART1) & 0xFF);
		Queue_EnQueue(usartRxQueue, data);
	}
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

void MyUSART_Init(usartFrameCallback handler){
	USART6_Init();//Tx init
	USART1_Init(handler); //Rx init
}

void USART_SendFrame(USART_FRAME frame){
	uint8_t startOfFrame = USART_FRAME_START_OF_FRAME;
	USART_SendByte(startOfFrame);
	USART_SendByte(frame.length);
	uint8_t byte1 = (frame.nodeId >> 8) & 0xFF, byte2 = (frame.nodeId & 0xFF);
	USART_SendByte(byte1);
	USART_SendByte(byte2);
	USART_SendByte(frame.endpoint);
	USART_SendByte(frame.id);
	USART_SendByte(frame.type);
	USART_SendByte(frame.payloadLength);
	for(uint8_t i = 0; i < frame.payloadLength; i++){
		USART_SendByte(frame.payload[i]);
	}
	USART_SendByte(frame.sequence);
	USART_SendByte(frame.cxor);
}

USART_FRAME USART_GenerateFrame(uint16_t nodeId, uint8_t endpoint, uint8_t cmdId, uint8_t cmdType, uint8_t payloadLength, uint8_t *payload){
	static uint8_t sequence = 0;
	USART_FRAME retFrame = USART_FRAME_DEFAULT;
	retFrame.length = payloadLength + 7;
	retFrame.nodeId = nodeId;
	retFrame.endpoint = endpoint;
	retFrame.id = cmdId;
	retFrame.type= cmdType;
	retFrame.payloadLength = payloadLength;
	if(payload != NULL){
		for(uint8_t i = 0; i < payloadLength; i++){
		  retFrame.payload[i] = payload[i];
		}
	}
	retFrame.sequence = sequence++;
	retFrame.cxor = CalcXor(retFrame);
	return retFrame;
}

USART_FRAME USART_GenerateDeviceConnected(void){
	return USART_GenerateFrame(NODE_ID_NOT_CARE,
							   ENDPOINT_NOT_CARE,
							   USART_FRAME_CMD_ID_DEVICE_CONNECTED,
							   USART_FRAME_CMD_TYPE_RESPONSE,
							   0,
							   NULL);
}
USART_FRAME USART_GenerateDeviceDisconnected(void){
	return USART_GenerateFrame(NODE_ID_NOT_CARE,
							   ENDPOINT_NOT_CARE,
							   USART_FRAME_CMD_ID_DEVICE_DISCONNECTED,
							   USART_FRAME_CMD_TYPE_RESPONSE,
							   0,
							   NULL);
}
USART_FRAME USART_GenerateLevelControl(LEVEL_CONTROL_CMD_e levelCmd, uint8_t level){
	const uint8_t payloadLength = 2;
	uint8_t payload[payloadLength];
	payload[0] = (uint8_t)levelCmd;
	payload[1] = level;
	return USART_GenerateFrame(NODE_ID_NOT_CARE,
							   ENDPOINT_NOT_CARE,
							   USART_FRAME_CMD_ID_LEVEL_CONTROL,
							   USART_FRAME_CMD_TYPE_SET,
							   payloadLength,
							   payload);
}
USART_FRAME USART_GenerateDeviceReport(void){
	return USART_GenerateFrame(NODE_ID_NOT_CARE,
							   ENDPOINT_NOT_CARE,
							   USART_FRAME_CMD_ID_DEVICE_REPORT,
							   USART_FRAME_CMD_TYPE_GET,
							   0,
							   NULL);
}



static void USART_SendByte(uint8_t byte){
	while(USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
	USART_SendData(USART6, byte);
	while(USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
}
static uint8_t CalcXor(USART_FRAME frame){
	uint8_t crc = USART_FRAME_CHECKXOR_INIT_VALUE;
	uint8_t byte1 = 0, byte2 = 0;
	byte1 = (frame.nodeId >> 8) & 0xFF;
	byte2 = frame.nodeId & 0xFF;
//	crc ^= (uint8_t)frame.nodeId;
	crc ^= byte1;
	crc ^= byte2;
	crc ^= frame.endpoint;
	crc ^= frame.id;
	crc ^= frame.type;
	crc ^= frame.payloadLength;
	for(uint8_t i = 0; i < frame.payloadLength; i++){
		crc ^= frame.payload[i];
	}
	crc ^= frame.sequence;
	return crc;
}
static bool CheckXor(USART_FRAME frame){
	return CalcXor(frame) == frame.cxor;
}
static USART_FRAME PollRxBuffer(USART_TypeDef* USARTx){
	USART_FRAME retFrame = USART_FRAME_DEFAULT;
	if(USARTx == USART2){
		uint8_t data = 0;
		if(Queue_GetFront(usartRxQueue, &data) && data == 0xB1){
			//dequeue the start byte
			Queue_DeQueue(usartRxQueue, NULL);
			Queue_DeQueue(usartRxQueue, &retFrame.length);
			uint8_t byte1 = 0, byte2 = 0;
			Queue_DeQueue(usartRxQueue, &byte1);
			Queue_DeQueue(usartRxQueue, &byte2);
			retFrame.nodeId = (uint16_t)((byte1 << 8) | byte2);
			Queue_DeQueue(usartRxQueue, &retFrame.endpoint);
			Queue_DeQueue(usartRxQueue, &retFrame.id);
			Queue_DeQueue(usartRxQueue, &retFrame.type);
			Queue_DeQueue(usartRxQueue, &retFrame.payloadLength);
			for(uint8_t i = 0; i < retFrame.payloadLength; i++){
				Queue_DeQueue(usartRxQueue, &retFrame.payload[i]);
			}
			Queue_DeQueue(usartRxQueue, &retFrame.sequence);
			Queue_DeQueue(usartRxQueue, &retFrame.cxor);
			if(CheckXor(retFrame)) {
				return retFrame;
			}
			else{ //checkxor fail, reset the length
				usartRxQueue->frontPos = 0;
				usartRxQueue->backPos = 0;
				retFrame.length = 0;
				return retFrame;
			}

		}
		else{
			Queue_DeQueue(usartRxQueue, NULL);
		}
	}
	else{

	}
	return retFrame;
}
static void ProcessSerialReceiver(void){
	//scan usart1 buffer
	USART_FRAME frame = USART_FRAME_DEFAULT;
	frame = PollRxBuffer(USART2);
	if(frame.length != 0){ // command received
		if(usartRxHandler != NULL){
			usartRxHandler(frame);
		}
	}

	//scan others port buffer
}


static void USART1_Init(usartFrameCallback handler)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(USART1_GPIO_RCC, ENABLE);
	RCC_APB2PeriphClockCmd(USART1_CLOCK, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = USART1_RX_GPIO_PIN;

	GPIO_Init(USART1_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(USART1_GPIO_PORT, GPIO_PinSource7, GPIO_AF_USART1);

	USART_InitStructure.USART_BaudRate = USART_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);

	//config interrupt when usart6 transmit data
	//enable usartx receive interrupt
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//nvic configuration
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);


	usartRxHandler = handler;

	usartRxQueue = Queue_Init(128);

	s_idProcessSerialTimer = TimerStart("ProcessSerialReceiver",
				200, TIMER_REPEAT_FOREVER,
				(void*)ProcessSerialReceiver, NULL);
}
static void USART6_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;

	RCC_AHB1PeriphClockCmd(GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(USART6_CLOCK, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = USART6_TX_GPIO_PIN;

	GPIO_Init(USART6_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(USART6_GPIO_PORT, GPIO_PinSource6, GPIO_AF_USART6);

	//config for usart6
	USART_InitStructure.USART_BaudRate = USART_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;

	USART_Init(USART6, &USART_InitStructure);
	USART_Cmd(USART6, ENABLE);
}

/*
 * USART.c
 *
 *  Created on: Jan 31, 2024
 *      Author: Administrator
 */

#include "USART.h"


usartFrameCallback usart1FrameCB = NULL;
EmberEventControl usart1ScanEventControl;
EmberEventControl processSerialEventControl;
queue_p usart1RxQueue;

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
static USART_FRAME PollRxBuffer(uint8_t port){
	USART_FRAME retFrame = USART_FRAME_DEFAULT;
	if(port == COM_USART1){
		uint8_t data = 0;
		if(Queue_GetFront(usart1RxQueue, &data) && data == 0xB1){
			//dequeue the start byte
			Queue_DeQueue(usart1RxQueue, NULL);
			Queue_DeQueue(usart1RxQueue, &retFrame.length);
			uint8_t byte1 = 0, byte2 = 0;
			Queue_DeQueue(usart1RxQueue, &byte1);
			Queue_DeQueue(usart1RxQueue, &byte2);
			retFrame.nodeId = (uint16_t)((byte1 << 8) | byte2);
			Queue_DeQueue(usart1RxQueue, &retFrame.endpoint);
			Queue_DeQueue(usart1RxQueue, &retFrame.id);
			Queue_DeQueue(usart1RxQueue, &retFrame.type);
			Queue_DeQueue(usart1RxQueue, &retFrame.payloadLength);
			for(uint8_t i = 0; i < retFrame.payloadLength; i++){
				Queue_DeQueue(usart1RxQueue, &retFrame.payload[i]);
			}
			Queue_DeQueue(usart1RxQueue, &retFrame.sequence);
			Queue_DeQueue(usart1RxQueue, &retFrame.cxor);
//			//print the frame
//			emberAfCorePrint("Frame: ");
//			emberAfCorePrint("%d ", retFrame.length);
//			emberAfCorePrint("%d ", retFrame.zigbeeData.nodeId);
//			emberAfCorePrint("%d ", retFrame.zigbeeData.endpoint);
//			emberAfCorePrint("%d ", retFrame.id);
//			emberAfCorePrint("%d ", retFrame.type);
//			emberAfCorePrint("%d ", retFrame.payloadLength);
//			for(uint8_t i = 0; i < retFrame.payloadLength; i++){
//				emberAfCorePrint("%d ", retFrame.payload[i]);
//			}
//			emberAfCorePrint("%d ", retFrame.sequence);
//			emberAfCorePrint("%d ", retFrame.cxor);
//			emberAfCorePrint("%d \n", CalcXor(retFrame));
			if(CheckXor(retFrame)) {
				return retFrame;
			}
			else{ //checkxor fail, reset the length
				emberAfCorePrintln("Frame ERROR");
				usart1RxQueue->frontPos = 0;
				usart1RxQueue->backPos = 0;
				retFrame.length = 0;
				return retFrame;
			}

		}
		else{
			Queue_DeQueue(usart1RxQueue, NULL);
		}
	}
	else{

	}
	return retFrame;
}

static void ProcessSerialReceiver(void){
	//scan usart1 buffer
	USART_FRAME frame = USART_FRAME_DEFAULT;
	frame = PollRxBuffer(COM_USART1);
	if(frame.length != 0){ // command received
		usart1FrameCB(frame);
	}

	//scan others port buffer
}


/****************************************************************************
 * 							PUBLIC MEMBER									*
 ****************************************************************************/
void USART1_Init(usartFrameCallback cb){
	//usart1: rx: PA3
	//usart1: tx: PA4
	usart1RxQueue = Queue_Init(USART1_QUEUE_BUFFER_SIZE);
	emberSerialInit(COM_USART1, 115200, PARITY_NONE, 1);

	usart1FrameCB = cb;
	//start the scan event, late delay at the first call: ready for peripherals to init
	emberEventControlSetDelayMS(usart1ScanEventControl, USART1_SCAN_LATE_DELAY_INTERVAL);
	emberEventControlSetDelayMS(processSerialEventControl, USART1_SCAN_LATE_DELAY_INTERVAL);
}

void USART_SendFrame(uint8_t port, USART_FRAME frame){
	//transmit the start byte
	uint8_t startByte = USART_FRAME_START_OF_FRAME;
	emberSerialWriteData(COM_USART1, &startByte, 1);
	//transmit the frame
	emberSerialWriteData(COM_USART1, &frame.length, 1);
	uint8_t byte1 = (uint8_t)((frame.nodeId >> 8) & 0xFF), byte2 = (uint8_t)(frame.nodeId & 0xFF);
	emberSerialWriteData(COM_USART1, &byte1, 1);
	emberSerialWriteData(COM_USART1, &byte2, 1);
	emberSerialWriteData(COM_USART1, &frame.endpoint, 1);
	emberSerialWriteData(COM_USART1, &frame.id, 1);
	emberSerialWriteData(COM_USART1, &frame.type, 1);
	emberSerialWriteData(COM_USART1, &frame.payloadLength, 1);
	for(uint8_t i = 0; i < frame.payloadLength; i++){
		emberSerialWriteData(COM_USART1, (&frame.payload[i]), 1);
	}
	emberSerialWriteData(COM_USART1, &frame.sequence, 1);
	emberSerialWriteData(COM_USART1, &frame.cxor, 1);
}

USART_FRAME USART_GenerateFrame(EmberNodeId nodeId,
								uint8_t endpoint,
								uint8_t cmdId,
								uint8_t cmdType,
								uint8_t payloadLength,
								uint8_t *payload){
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
USART_FRAME USART_GenerateLevelControl(EmberNodeId nodeId, uint8_t endpoint, uint8_t level){
	const uint8_t payloadLength = 1;
	uint8_t payload[payloadLength];
	payload[0] = level;
	return USART_GenerateFrame(nodeId,
							   endpoint,
							   USART_FRAME_ID_LEVEL_CONTROL,
							   USART_FRAME_TYPE_RESPONSE,
							   payloadLength,
							   payload);
}

USART_FRAME USART_GenerateDeviceConnected(EmberNodeId nodeId){
	return USART_GenerateFrame(nodeId, 0x01, USART_FRAME_ID_DEVICE_CONNECTED, USART_FRAME_TYPE_RESPONSE, 0, NULL);
}
USART_FRAME USART_GenerateDeviceDisconnected(EmberNodeId nodeId){
	return USART_GenerateFrame(nodeId, 0x01, USART_FRAME_ID_DEVICE_DISCONNECTED, USART_FRAME_TYPE_RESPONSE, 0, NULL);
}
USART_FRAME USART_GenerateDeviceReport(EmberNodeId nodeId, DeviceType_e type, ConnectState_e state){
	const uint8_t payloadLength = 2;
	uint8_t payload[payloadLength];
	payload[0] = (uint8_t)type;
	payload[1] = (uint8_t)state;
	return USART_GenerateFrame(nodeId, 0x01, USART_FRAME_ID_NETWORK_REPORT, USART_FRAME_TYPE_RESPONSE, payloadLength, payload);
}
USART_FRAME USART_GenerateDeviceJoinNetwork(EmberNodeId nodeId, DeviceType_e type){
	const uint8_t payloadLength = 1;
	uint8_t payload[payloadLength];
	payload[0] = type;
	return USART_GenerateFrame(nodeId, 0x01, USART_FRAME_ID_DEVICE_JOIN_NETWORK, USART_FRAME_TYPE_RESPONSE, payloadLength, payload);
}
USART_FRAME USART_GenerateDeviceLeaveNetwork(EmberNodeId nodeId){
	return USART_GenerateFrame(nodeId, 0x01, USART_FRAME_ID_DEVICE_LEAVE_NETWORK, USART_FRAME_TYPE_RESPONSE, 0, NULL);
}
USART_FRAME USART_GenerateOpenNetwork(void){
	EmberNodeId nodeId = DeviceManager_GetDeviceType(HC);
	return USART_GenerateFrame( nodeId,
								0x01,
								USART_FRAME_ID_NETWORK_OPENING,
								USART_FRAME_TYPE_RESPONSE,
								0,
								NULL);
}
USART_FRAME USART_GenerateCloseNetwork(void){
	EmberNodeId nodeId = DeviceManager_GetDeviceType(HC);
	return USART_GenerateFrame( nodeId,
								0x01,
								USART_FRAME_ID_NETWORK_CLOSING,
								USART_FRAME_TYPE_RESPONSE,
								0,
								NULL);
}
USART_FRAME USART_GenerateCreateNetwork(void){
	return USART_GenerateFrame( HC,
								0x01,
								USART_FRAME_ID_NETWORK_CREATING,
								USART_FRAME_TYPE_RESPONSE,
								0,
								NULL);
}


/****************************************************************************
 * 							EVENT HANDLER									*
 ****************************************************************************/
void usart1ScanEventHandler(void){
	emberEventControlSetInactive(usart1ScanEventControl);

	uint16_t numOfByteAvail = emberSerialReadAvailable(COM_USART1);
	if(numOfByteAvail > 0){
		uint8_t data;
		emberSerialReadByte(COM_USART1, &data);
		Queue_EnQueue(usart1RxQueue, data);
	}
	emberEventControlSetDelayMS(usart1ScanEventControl, USART1_SCAN_INTERVAL);
}

void processSerialEventHandler(void){
	emberEventControlSetInactive(processSerialEventControl);
	ProcessSerialReceiver();
	emberEventControlSetDelayMS(processSerialEventControl, USART_PROCESS_SERIAL_INTERVAL);
}

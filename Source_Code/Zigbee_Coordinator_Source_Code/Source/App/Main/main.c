/*
 * main.c
 *
 *  Created on: Feb 20, 2024
 *      Author: Administrator
 */
#include "main.h"

//#include "custom_token.h"
#include <string.h>

#define LED_UX_NUMBER								LED_2
#define LED_TOGGLE_SLOW_ON_TIME_MS					400
#define LED_TOGGLE_SLOW_OFF_TIME_MS					600
#define LED_TOGGLE_FAST_ON_TIME_MS					200
#define LED_TOGGLE_FAST_OFF_TIME_MS					300
#define LED_TOGGLE_SLOW_PAIR						LED_TOGGLE_SLOW_ON_TIME_MS, LED_TOGGLE_SLOW_OFF_TIME_MS
#define LED_TOGGLE_FAST_PAIR						LED_TOGGLE_FAST_ON_TIME_MS, LED_TOGGLE_FAST_OFF_TIME_MS

#define ESP32_USART_COM								COM_USART1

#define DEVICE_MANAGER_EVENT_CONTROL_INTERVAL		1000
#define DEVICE_OFFLINE_THRESHHOLD_INTERVAL			((uint32_t)(16 * 1000 * 60)) ///15 minutes

typedef enum{
	POWER_ON,
	REBOOT,
	IDLE
}MAIN_STATE_e;
EmberEventControl mainStateEventControl;
EmberEventControl deviceManagerEventControl;
MAIN_STATE_e mainState;


void emberAfMainInitCallback(void)
{
	Main_Init();
}
void emberIncomingManyToOneRouteRequestHandler(EmberNodeId source,
                                               EmberEUI64 longId,
                                               uint8_t cost)
{
	emberAfCorePrintln("Many to one route request receive from nodeId :%d", source);
}

void Main_Init(void){
	Timer_Init(10);
	LedControl_Init();
	USART1_Init(Main_Usart1Handle);
	ButtonControl_Init(Main_ButtonPressHandle, NULL);

	Network_Init(Main_NetworkHandle);
	Receive_Init(Main_ReceiveHandle);
	emberAfCorePrintln("Main Init");
	mainState = POWER_ON;
	emberEventControlSetDelayMS(mainStateEventControl, 1000);
	emberEventControlSetDelayMS(deviceManagerEventControl, DEVICE_MANAGER_EVENT_CONTROL_INTERVAL);
}


void mainStateEventHandler(void){
	switch(mainState){
	case POWER_ON:
	{
		//call Main_ReceiveHandle manually to tell the app that HC is now online
		Main_ReceiveHandle(0x0000, DEVICE_CONNECTED, 0, NULL);
		mainState = IDLE;
		break;
	}
	case IDLE:
	{
		break;
	}
	case REBOOT:
	{
		halReboot();
		break;
	}
	default:
		break;
	}
}

//managing devices:
//	+ if device has not communicate with HC after 15 minutes passed, marks device as offline and send response to server
void deviceManagerEventHandler(void){
	emberEventControlSetInactive(deviceManagerEventControl);

	//check any device has not communicate after 15m
	Device_s *list = DeviceManager_GetDeviceList();
	uint8_t length = DeviceManager_GetDeviceListLength();
	for(uint8_t i = 1; i < length; i++){
		//get last time
		uint32_t lastTime = list[i].lastTime;
		uint32_t timeCurrent = Timer_GetCounter();
		ConnectState_e connection = list[i].state;
		if(Timer_CalcTimeDiff(lastTime, timeCurrent) >= DEVICE_OFFLINE_THRESHHOLD_INTERVAL && connection == ONLINE){
			//marks device as offline
			DeviceManager_SetDeviceOffline(list[i].nodeId);
			//and send response to HC
			Main_ReceiveHandle(list[i].nodeId, DEVICE_DISCONNECTED, 0, NULL);
		}
	}

	emberEventControlSetDelayMS(deviceManagerEventControl, DEVICE_MANAGER_EVENT_CONTROL_INTERVAL);
}

void Main_Usart1Handle(USART_FRAME frame){
	//special frame
	if(frame.id == USART_FRAME_ID_NETWORK_CREATING && frame.type == USART_FRAME_TYPE_SET){
		Network_CreateNetwork();
		//send a response to the server
		USART_FRAME frame = USART_GenerateCreateNetwork();
		USART_SendFrame(ESP32_USART_COM, frame);
	}
	else{
		EmberNodeId nodeId = frame.nodeId;
		DeviceType_e type = DeviceManager_GetDeviceType(nodeId);
		if(type == HC){
			emberAfCorePrintln("Frame Handle");
			switch(frame.id){
			case USART_FRAME_ID_NETWORK_OPENING:
			{
				if(frame.type == USART_FRAME_TYPE_SET){
					Network_OpenNetwork();
				}
				break;
			}
			case USART_FRAME_ID_NETWORK_CLOSING:
			{
				if(frame.type == USART_FRAME_TYPE_SET){
					Network_CloseNetwork();
				}
				break;
			}
			case USART_FRAME_ID_NETWORK_REPORT:
			{
				if(frame.type == USART_FRAME_TYPE_GET){
					Main_NetworkSync();
				}
				break;
			}
			default:
				break;
			}
		}
		else{//forward to the device with associated node id
			//if only the device is now online
			if(DeviceManager_GetDeviceConnectionState(nodeId) == ONLINE){
				switch(frame.id){
				emberAfCorePrintln("Forward message to device with id: 0x%x", nodeId);
				case USART_FRAME_ID_LEVEL_CONTROL:
				{
					//fill level control cluster
					Send_LevelControl(nodeId, 0x01, frame.endpoint, frame.payload[0]);
					break;
				}
				default:
					break;
				}
			}


		}
	}
}

void Main_ButtonPressHandle(uint8_t index){
	PRESS_EVENT_e event = ButtonControl_GetPressEvent(index);
	emberAfCorePrintln("Button %d press %d time(s)", index, event);
	if(index == 0){//Press B1
		switch(event){
		case press_1:
		{
			Network_OpenNetwork();
			//update network state to server
			USART_FRAME frame = USART_GenerateOpenNetwork();
			USART_SendFrame(ESP32_USART_COM, frame);
			emberAfCorePrintln("Open frame sent");
			break;
		}
		case press_3:
		{
			Network_CreateNetwork();
			break;
		}
		case press_5:
		{
			mainState = REBOOT;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;
		}
		default:
			break;
		}
	}
	else if(index == 1){//Press B2
		switch(event){
		case press_1:
		{
			Network_CloseNetwork();
			USART_FRAME frame = USART_GenerateCloseNetwork();
			USART_SendFrame(ESP32_USART_COM, frame);
			emberAfCorePrintln("Close frame sent");
			break;
		}
		default:
			break;
		}
	}
}


//handle receive zcl from device in the network
void Main_ReceiveHandle(EmberNodeId nodeId, RECEIVE_CMD_ID_e receiveId, uint8_t length, uint8_t *payload){
	bool cmdHandled = false;
	switch(receiveId){
	case DEVICE_JOIN_NETWORK:
	case DEVICE_CONNECTED:
	{
		//receive modelID when device join or reconnect to the network
		cmdHandled = true;
		Device_s device = DeviceManager_AddDevice(nodeId, payload, length);
		if(DeviceManager_IsValidDevice(device)){//device not exist in the list: join network -> send join network frame to ESP32
			DeviceType_e type = DeviceManager_GetDeviceType(nodeId);
			emberAfCorePrintln("Device join network with id: 0x%x, type: %d", nodeId, type);
			USART_FRAME frame = USART_GenerateDeviceJoinNetwork(nodeId, type);
			USART_SendFrame(ESP32_USART_COM, frame);
		}
		else{//already in the list: rejoin -> send connected frame to ESP32
			emberAfCorePrintln("Device connected with id: 0x%x", nodeId);
			DeviceManager_SetDeviceOnline(nodeId);
			USART_FRAME frame = USART_GenerateDeviceConnected(nodeId);
			USART_SendFrame(ESP32_USART_COM, frame);
		}

		break;
	}
	case LEVEL_CONTROL:
	{
		cmdHandled = true;
		uint8_t level = payload[0];
		emberAfCorePrintln("Level control frame receive from %x with value %d", nodeId, level);
		USART_FRAME frame = USART_GenerateLevelControl(nodeId, 0x01, level);
		USART_SendFrame(ESP32_USART_COM, frame);
		break;
	}
	case DEVICE_DISCONNECTED:
	{
		cmdHandled = true;
		emberAfCorePrintln("Device disconnected with id: 0x%x", nodeId);
		DeviceManager_SetDeviceOffline(nodeId);
		USART_FRAME frame = USART_GenerateDeviceDisconnected(nodeId);
		USART_SendFrame(ESP32_USART_COM, frame);
		break;
	}
	case DEVICE_LEAVE_NETWORK:
	{
		emberAfCorePrintln("Device leave network with id: 0x%x", nodeId);
		//erase device from list
		DeviceManager_DeleteDevice(nodeId);
		USART_FRAME frame = USART_GenerateDeviceLeaveNetwork(nodeId);
		USART_SendFrame(ESP32_USART_COM, frame);
		break;
	}
	default:
		break;
	}

	//save last communicate time
	if(cmdHandled){
		uint32_t time = Timer_GetCounter();
		DeviceManager_UpdateLastTime(nodeId, time);
	}
}

void Main_NetworkHandle(NETWORK_STATE_e state){
	switch(state){
	case NETWORK_OPENING:
	{
		emberAfCorePrintln("Network opening");
		if(emberAfNetworkState() != EMBER_JOINED_NETWORK){ // chua khoi tao mang
			LedControl_Toggle(LED_UX_NUMBER, RED, 3, LED_TOGGLE_SLOW_PAIR);
		}
		else{
			LedControl_Toggle(LED_UX_NUMBER, GREEN, 3, LED_TOGGLE_SLOW_PAIR);
		}
		break;
	}
	case NETWORK_CLOSING:
	{
		emberAfCorePrintln("Network closing");
		if(emberAfNetworkState() != EMBER_JOINED_NETWORK){ // chua khoi tao mang
			LedControl_Toggle(LED_UX_NUMBER, RED, 3, LED_TOGGLE_SLOW_PAIR);
		}
		else{
			LedControl_Toggle(LED_UX_NUMBER, GREEN, 3, LED_TOGGLE_SLOW_PAIR);
		}
		break;
	}
	case NETWORK_CREATING:
	{
		emberAfCorePrintln("Network creating");
		LedControl_Toggle(LED_UX_NUMBER, GREEN, 5, LED_TOGGLE_SLOW_PAIR);
		break;
	}
	default:
		break;
	}
}
void Main_NetworkSync(void){
	//send a network report with type set to erase the database in esp32
	USART_FRAME frame = USART_GenerateFrame(HC, 0x01, USART_FRAME_ID_NETWORK_REPORT, USART_FRAME_TYPE_SET, 0, NULL);
	USART_SendFrame(ESP32_USART_COM, frame);
	emberAfCorePrintln("Network report response");
	//update the current network state of all devices currently in the network
	//such as: device id + device type + device connection state for ESP32 to allocate pins
	Device_s* list = DeviceManager_GetDeviceList();
	uint8_t length = DeviceManager_GetDeviceListLength();
	for(uint8_t i = 0; i < length; i++){
		Device_s device = *(list + i);
		EmberNodeId nodeId = device.nodeId;
		USART_FRAME frame = USART_GenerateDeviceReport(nodeId, device.type, device.state);
		USART_SendFrame(ESP32_USART_COM, frame);
	}
}

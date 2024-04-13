/*
 * main.c
 *
 *  Created on: Jan 25, 2024
 *      Author: Administrator
 */
#include "app/framework/include/af.h"
//import standard libraries
#include <stdint.h>
#include <stdbool.h>
//import drivers
#include "Source/Mid/USART/USART.h"
//import middle-wares
#include "Source/Mid/Timer/Timer.h"
#include "Source/Mid/Button/Button.h"
#include "Source/Mid/Led/Led.h"
//import apps
#include "Source/App/Network/Network.h"
#include "Source/App/Send/Send.h"
#include "Source/App/Receive/Receive.h"
#include "Source/App/CurtainControl/CurtainControl.h"

#define STM32_USART_COM							COM_USART1
#define NETWORK_LEAVE_WAIT_INTERVAL				2000


#define LED_NETWORK_UX_NUMBER					LED_2
#define LED_USER_UX_NUMBER						LED_2
#define LED_TOGGLE_SLOW_ON_TIME_MS				400
#define LED_TOGGLE_SLOW_OFF_TIME_MS				600
#define LED_TOGGLE_FAST_ON_TIME_MS				200
#define LED_TOGGLE_FAST_OFF_TIME_MS				300
#define LED_TOGGLE_SLOW_PAIR					LED_TOGGLE_SLOW_ON_TIME_MS, LED_TOGGLE_SLOW_OFF_TIME_MS
#define LED_TOGGLE_FAST_PAIR					LED_TOGGLE_FAST_ON_TIME_MS, LED_TOGGLE_FAST_OFF_TIME_MS


#define UPDATE_HC_EVENT_INTERVAL				((uint32_t)(15 * 1000 * 60))

typedef enum{
	POWER_ON,
	IDLE,
	REBOOT,
	REBOOT_AND_LEAVE,
	LEAVE_NETWORK,
	WAIT_TO_LEAVE,
	WAIT_TO_LEAVE_AND_REBOOT,
	REPORT
}MAIN_STATE_e;

static void APP_pressEventHandler(uint8_t);
static void APP_holdEventHandler(uint8_t);
static void APP_networkHandler(NETWORK_STATE_e);
static void APP_usart1Handler(USART_FRAME);
static void APP_receiveHandler(RECEIVE_CMD_e, uint8_t, uint8_t*);
static void APP_curtainHandler(uint8_t level);


EmberEventControl mainStateEventControl;
EmberEventControl updateHCEventControl;
MAIN_STATE_e mainState;
static bool networkReady = false;


void emberAfMainInitCallback(void)
{
	Timer_Init(50);
	//driver init
	USART1_Init(APP_usart1Handler);
	//middle-ware init
	LedControl_Init();
	ButtonControl_Init(APP_pressEventHandler, APP_holdEventHandler);
	//app init
	NETWORK_Init(APP_networkHandler);
	Receive_Init(APP_receiveHandler);
	CurtainControl_Init(APP_curtainHandler);

	mainState = POWER_ON;
	emberEventControlSetDelayMS(mainStateEventControl, 1000);
	emberEventControlSetDelayMS(updateHCEventControl, UPDATE_HC_EVENT_INTERVAL);
	emberAfCorePrintln("Main init");
}


void mainStateEventHandler(void){
	emberEventControlSetInactive(mainStateEventControl);
	switch(mainState){
	case POWER_ON:
	{
		mainState = IDLE;
		//update current level to the controller (STM32)
		CurtainControl_Update();


		EmberNetworkStatus nwkCurrentStatus = emberAfNetworkState();
		if(nwkCurrentStatus == EMBER_NO_NETWORK){
			LedControl_Toggle(LED_NETWORK_UX_NUMBER, RED, 2, LED_TOGGLE_SLOW_PAIR);
		}
		break;
	}
	case REPORT:
	{
		mainState = IDLE;
		SEND_ReportInfoToHC();
		break;
	}
	case REBOOT:
	{
		mainState = IDLE;
		halReboot();
		break;
	}
	case LEAVE_NETWORK:
	{
		mainState = WAIT_TO_LEAVE;
		SEND_ZigDevRequest();
		emberEventControlSetDelayMS(mainStateEventControl, NETWORK_LEAVE_WAIT_INTERVAL);
		break;
	}
	case REBOOT_AND_LEAVE:
	{
		mainState = WAIT_TO_LEAVE_AND_REBOOT;
		SEND_ZigDevRequest();
		emberEventControlSetDelayMS(mainStateEventControl, NETWORK_LEAVE_WAIT_INTERVAL);
		break;
	}
	case WAIT_TO_LEAVE:
	{
		emberClearBindingTable();
		emberLeaveNetwork();
		break;
	}
	case WAIT_TO_LEAVE_AND_REBOOT:
	{
		emberClearBindingTable();
		emberLeaveNetwork();
		halReboot();
		break;
	}
	case IDLE:
	{
		break;
	}
	default:
		break;
	}
}

void updateHCEventHandler(void){
	emberEventControlSetInactive(updateHCEventControl);

	//send all information that needed to be update to HC
	CurtainControl_Update();

	emberEventControlSetDelayMS(updateHCEventControl, UPDATE_HC_EVENT_INTERVAL);
}

static void APP_networkHandler(NETWORK_STATE_e state){
	switch (state){
	//find network
	case NETWORK_JOIN_SUCCESS:
	{
		LedControl_Toggle(LED_NETWORK_UX_NUMBER, GREEN, 2, LED_TOGGLE_SLOW_PAIR);
		networkReady = true;
		mainState = REPORT;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;
	}
	case NETWORK_JOIN_FAIL:
	{
		LedControl_Toggle(LED_NETWORK_UX_NUMBER, RED, 1, LED_TOGGLE_SLOW_PAIR);
		mainState = IDLE;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;
	}
	case NETWORK_FINDING_NETWORK:
	{
		LedControl_Toggle(LED_NETWORK_UX_NUMBER, BLUE, 5, LED_TOGGLE_SLOW_PAIR);
		break;
	}
	//rejoin the network
	case NETWORK_HAS_PARENT:
	{
		LedControl_Toggle(LED_NETWORK_UX_NUMBER, GREEN, 2, LED_TOGGLE_SLOW_PAIR);
		networkReady = true;
		mainState = REPORT;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;
	}
	case NETWORK_LOST_PARENT:
	{
		LedControl_Toggle(LED_NETWORK_UX_NUMBER, BLUE, 2, LED_TOGGLE_SLOW_PAIR);
		mainState = IDLE;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;
	}

	//leave network
	case NETWORK_OUT_NETWORK:
	{
		LedControl_Toggle(LED_NETWORK_UX_NUMBER, GREEN, 1, LED_TOGGLE_SLOW_PAIR);
		mainState = REBOOT;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;
	}

	default:
		break;
	}
}
static void APP_usart1Handler(USART_FRAME frame){
	emberAfCorePrintln("Frame Received with id: %d", frame.id);
	switch(frame.id){
	case USART_FRAME_ID_DEVICE_CONNECTED:
	{
		//STM32 is now online, send update information to STM32
		//send connected frame first
		USART_FRAME frame = USART_GenerateDeviceConnected();
		USART_SendFrame(STM32_USART_COM, frame);
		//then send update info
		frame = USART_GenerateLevelControl(LEVEL_SET, CurtainControl_GetCurrentLevel());
		USART_SendFrame(STM32_USART_COM, frame);
		break;
	}
	case USART_FRAME_ID_LEVEL_CONTROL:
	{
		//receive level control frame from STM32, handle and send response to HC
		if(frame.payloadLength >= 2){
			switch((LEVEL_CONTROL_CMD_e)frame.payload[0]){
			case LEVEL_SET:
			{
				CurtainControl_SetLevel(LEVEL_SET, frame.payload[1]);
				break;
			}
			case LEVEL_INCREASE:
			{
				CurtainControl_IncreaseLevel();
				break;
			}
			case LEVEL_DECREASE:
			{
				CurtainControl_DecreaseLevel();
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	}
}
static void APP_receiveHandler(RECEIVE_CMD_e receiveCmd, uint8_t length, uint8_t* payload){
	switch(receiveCmd){
	case LEVEL_CONTROL:
	{
		//receive ZCL level control
		uint8_t level = payload[0];
		CurtainControl_SetLevel(LEVEL_SET, level);
		break;
	}
	default:
		break;
	}
}

static void APP_curtainHandler(uint8_t level){
	//send report to hc
	SEND_LevelStateReport(0x01, level);

	//send response to the controller (STM32)
	USART_FRAME frame = USART_GenerateLevelControl(LEVEL_SET, level);
	USART_SendFrame(STM32_USART_COM, frame);
}

static void APP_pressEventHandler(uint8_t index){
	PRESS_EVENT_e event = ButtonControl_GetPressEvent(index);
	emberAfCorePrintln("Button %d press %d time(s)", index, event);
	if(index == 0){//sw1
		switch(event){
		case press_1:
		{
			//open curtain
			emberAfCorePrintln("Curtain open");
			CurtainControl_Open();
			if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
				LedControl_Toggle(LED_USER_UX_NUMBER, GREEN, 1, LED_TOGGLE_SLOW_PAIR);
			}
			else{
				LedControl_Toggle(LED_USER_UX_NUMBER, RED, 1, LED_TOGGLE_SLOW_PAIR);
			}
			break;
		}
		case press_3:
		{
			//join network
			NETWORK_FindAndJoin();
			break;
		}
		case press_5:
		{
			emberAfCorePrintln("Reboot and leave");
			//reboot and leave network
			LedControl_Toggle(LED_USER_UX_NUMBER, GREEN, 1, LED_TOGGLE_SLOW_PAIR);
			mainState = REBOOT_AND_LEAVE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;
		}
		default:
			break;
		}
	}
	else if(index == 1){//sw2
		switch(event){
		case press_1:
		{
			//close curtain
			emberAfCorePrintln("Curtain close");
			CurtainControl_Close();
			if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
				LedControl_Toggle(LED_USER_UX_NUMBER, GREEN, 1, LED_TOGGLE_SLOW_PAIR);
			}
			else{
				LedControl_Toggle(LED_USER_UX_NUMBER, RED, 1, LED_TOGGLE_SLOW_PAIR);
			}
			break;
		}
		case press_3:
		{
			emberAfCorePrintln("Leave network");
			//leave network
			mainState = LEAVE_NETWORK;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;
		}
		case press_5:
		{
			emberAfCorePrintln("Update to HC");
			emberEventControlSetInactive(updateHCEventControl);
			emberEventControlSetActive(updateHCEventControl);

			break;
		}

		default:
			break;
		}
	}
}

static void APP_holdEventHandler(uint8_t index){
	HOLD_EVENT_e event = ButtonControl_GetHoldEvent(index);
	if(index == 0){ // sw1
		//trigger every 500 ms
		//increase curtain level
		CurtainControl_IncreaseLevel();
		if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
			LedControl_Toggle(LED_USER_UX_NUMBER, GREEN, 1, LED_TOGGLE_FAST_PAIR);
		}
		else{
			LedControl_Toggle(LED_USER_UX_NUMBER, RED, 1, LED_TOGGLE_FAST_PAIR);
		}
		switch(event){
		default:
			break;
		}
	}
	else if(index == 1){//sw2
		//trigger every 500ms
		//decrease curtain level
		CurtainControl_DecreaseLevel();
		if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
			LedControl_Toggle(LED_USER_UX_NUMBER, GREEN, 1, LED_TOGGLE_FAST_PAIR);
		}
		else{
			LedControl_Toggle(LED_USER_UX_NUMBER, RED, 1, LED_TOGGLE_FAST_PAIR);
		}
		switch(event){
		default:
			break;
		}
	}
}



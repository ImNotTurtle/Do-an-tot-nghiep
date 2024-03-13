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
//import middle-wares
#include "Source/Mid/Timer/Timer.h"
#include "Source/Mid/Button/Button.h"
#include "Source/Mid/Led/Led.h"
//import apps
#include "Source/App/Network/Network.h"
#include "Source/App/Send/Send.h"
#include "Source/App/Receive/Receive.h"

#define NETWORK_LEAVE_WAIT_INTERVAL				2000


#define LED_NETWORK_UX_NUMBER					LED_1
#define LED_USER_UX_NUMBER						LED_2
#define LED_TOGGLE_SLOW_ON_TIME_MS				400
#define LED_TOGGLE_SLOW_OFF_TIME_MS				600
#define LED_TOGGLE_FAST_ON_TIME_MS				200
#define LED_TOGGLE_FAST_OFF_TIME_MS				300
#define LED_TOGGLE_SLOW_PAIR					LED_TOGGLE_SLOW_ON_TIME_MS, LED_TOGGLE_SLOW_OFF_TIME_MS
#define LED_TOGGLE_FAST_PAIR					LED_TOGGLE_FAST_ON_TIME_MS, LED_TOGGLE_FAST_OFF_TIME_MS

#define LEVEL_CONTROL_INCREASE_LEVEL			25
#define LEVEL_CONTROL_DECREASE_LEVEL			25

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
static uint8_t CurtainControl_GetCurrentLevel(void);
static void CurtainControl_Close(void);
static void CurtainControl_Open(void);
static void CurtainControl_SetLevel(uint8_t level);


EmberEventControl mainStateEventControl;
EmberEventControl updateHCEventControl;
MAIN_STATE_e mainState;
static bool networkReady = false;


void emberAfMainInitCallback(void)
{
	Timer_Init(50);
	//middle-ware init
	LedControl_Init();
	ButtonControl_Init(APP_pressEventHandler, APP_holdEventHandler);

	//app init
	NETWORK_Init(APP_networkHandler);
	emberEventControlSetActive(mainStateEventControl);
	emberEventControlSetDelayMS(updateHCEventControl, UPDATE_HC_EVENT_INTERVAL);
	emberAfCorePrintln("Main init");
}


void mainStateEventHandler(void){
	emberEventControlSetInactive(mainStateEventControl);
	switch(mainState){
	case POWER_ON:
	{
		mainState = IDLE;
		EmberNetworkStatus nwkCurrentStatus = emberAfNetworkState();
		if(nwkCurrentStatus == EMBER_NO_NETWORK){
			LedControl_Toggle(LED_1, RED, 2, LED_TOGGLE_SLOW_PAIR);
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
	uint8_t level = CurtainControl_GetCurrentLevel();
	SEND_LevelStateReport(0x01, level);

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

static void APP_pressEventHandler(uint8_t index){
	PRESS_EVENT_e event = ButtonControl_GetPressEvent(index);
	emberAfCorePrintln("Button %d press %d time(s)", index, event);
	if(index == 0){//sw1
		switch(event){
		case press_1:
		{
			//open curtain
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
			//leave network
			mainState = LEAVE_NETWORK;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
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
		if(event >= hold){
			emberAfCorePrintln("Button %d hold 0.5s trigger", index);
			uint8_t level = CurtainControl_GetCurrentLevel();
			level += LEVEL_CONTROL_INCREASE_LEVEL;
			if(level >= 0 && level <= 100){
				CurtainControl_SetLevel(level);
				if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
					LedControl_Toggle(LED_USER_UX_NUMBER, GREEN, 1, LED_TOGGLE_FAST_PAIR);
				}
				else{
					LedControl_Toggle(LED_USER_UX_NUMBER, RED, 1, LED_TOGGLE_FAST_PAIR);
				}
			}
		}
		switch(event){
		default:
			break;
		}
	}
	else if(index == 1){//sw2
		//trigger every 500ms
		if(event > hold){
			emberAfCorePrintln("Button %d hold 0.5s trigger", index);
			uint8_t level = CurtainControl_GetCurrentLevel();
			level -= LEVEL_CONTROL_DECREASE_LEVEL;
			if(level >= 0 && level <= 100){
				CurtainControl_SetLevel(level);
				if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
					LedControl_Toggle(LED_USER_UX_NUMBER, GREEN, 1, LED_TOGGLE_FAST_PAIR);
				}
				else{
					LedControl_Toggle(LED_USER_UX_NUMBER, RED, 1, LED_TOGGLE_FAST_PAIR);
				}
			}
		}
		switch(event){
		default:
			break;
		}
	}
}


static uint8_t CurtainControl_GetCurrentLevel(void){
	uint8_t level = 0;
	emberAfReadServerAttribute(0x01, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, &level, 1);
	return level;
}
static void CurtainControl_Close(void){
	CurtainControl_SetLevel(0);
}
static void CurtainControl_Open(void){
	CurtainControl_SetLevel(100);
}
static void CurtainControl_SetLevel(uint8_t level){
	emberAfCorePrintln("current curtain level: %d", level);
	//send report to hc
	if(emberAfNetworkState() == EMBER_JOINED_NETWORK){
		SEND_LevelStateReport(0x01, level);
	}
}

#include <stdint.h>
#include <stdio.h>
#include "timer.h"
#include "USART.h"
#include "LCD.h"
#include "button.h"
#include "eventbutton.h"
//#include "eventman.h"
#include "DeviceManager.h"



/* PRIVATE VARIABLE-----------------------------------------------------------------*/
static void Main_UsartHandler(USART_FRAME frame);
static void Main_AppCommonInit(void);
static void Main_DeviceStateMachine(uint8_t event);
static void Main_DeviceManaging(void);


int main(void)
{
	Main_AppCommonInit();

	while(1)
	{
		processTimerScheduler();
		processEventScheduler();
	}
}

static void Main_AppCommonInit(void){
	SystemCoreClockUpdate();
	TimerInit();

	LCD_Init();

	Button_Init();
	EventButton_Init();
	EventSchedulerInit(Main_DeviceStateMachine);
	MyUSART_Init(Main_UsartHandler);

	DeviceManager_DeviceInit(Main_DeviceManaging, REM);

	//send controller connected frame to device
	USART_FRAME frame = USART_GenerateDeviceConnected();
	USART_SendFrame(frame);
}

static void Main_UsartHandler(USART_FRAME frame){
	switch(frame.id){
	case USART_FRAME_CMD_ID_DEVICE_CONNECTED:
	{
		if(frame.type == USART_FRAME_CMD_TYPE_RESPONSE){
			DeviceManager_SetDeviceOnline();
		}
		break;
	}
	case USART_FRAME_CMD_ID_LEVEL_CONTROL:
	{
		uint8_t level = frame.payload[1];
		DeviceManager_SetDeviceLevel(level);
		break;
	}
	default:
		break;
	}
}
static void Main_DeviceStateMachine(uint8_t event){
	switch(event){
	case EVENT_OF_BUTTON_2_PRESS_LOGIC:
	{
		USART_FRAME frame = USART_GenerateLevelControl(LEVEL_SET, 100);
		USART_SendFrame(frame);
		break;
	}
	case EVENT_OF_BUTTON_4_PRESS_LOGIC:
	{
		USART_FRAME frame = USART_GenerateLevelControl(LEVEL_SET, 0);
		USART_SendFrame(frame);
		break;
	}
	default:
		break;
	}
}
static void Main_DeviceManaging(void){
	//handle all event related to the device
	DEVICE_s device = DeviceManager_GetDevice();
	LCD_ClearScreen();
	//handle device connection first
	switch(device.state){
	case ONLINE:
	{
		LCD_DisplayState(ONLINE);
		break;
	}
	case OFFLINE:
	case UNKNOWN:
	{
		LCD_DisplayState(OFFLINE);
		break;
	}
	default:
		break;
	}

	//handle level change
	LCD_DisplayLevel(device.level);

	//save the last communicate time
	uint32_t currentTime = GetMilSecTick();
	DeviceManager_UpdateLastTime(currentTime);
}

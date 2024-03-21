/*
 * DeviceManager.c
 *
 *  Created on: Mar 16, 2024
 *      Author: Administrator
 */
#include "DeviceManager.h"

static DEVICE_s s_device;
static SSwTimer s_idUpdateTimer = NO_TIMER;
static deviceManagingHandler s_deviceHandler = NULL;

static bool DeviceManager_IsUpdatePending(void);
static void DeviceManager_SetUpdatePending(void);
static void DeviceManager_ClearUpdatePending(void);



void DeviceManager_DeviceInit(deviceManagingHandler handler, DEVICE_TYPE_e type, uint8_t level){
	s_device.type = type;
	s_device.level = level;
	s_device.state = UNKNOWN;
	s_device.lastTime = 0;
	s_device.updatePending = false;
	s_deviceHandler = handler;

	//timer with 1 minute cycle to check for update
	s_idUpdateTimer = TimerStart("DeviceManager_Update",
								  1000 * 60,
								  TIMER_REPEAT_FOREVER,
								  DeviceManager_Update,
								  NULL);
}
DEVICE_s DeviceManager_GetDevice(void){
	return s_device;
}
void DeviceManager_SetDeviceOnline(void){
	s_device.state = ONLINE;
	if(s_deviceHandler != NULL){
		s_deviceHandler();
	}
}
void DeviceManager_SetDeviceOffline(void){
	s_device.state = OFFLINE;
	if(s_deviceHandler != NULL){
		s_deviceHandler();
	}
}
void DeviceManager_SetDeviceLevel(uint8_t level){
	s_device.level = level;
	if(s_deviceHandler != NULL){
		s_deviceHandler();
	}
}
void DeviceManager_UpdateLastTime(uint32_t time){
	s_device.lastTime = time;

	//clear any pending update
	DeviceManager_ClearUpdatePending();
}
void DeviceManager_Update(void){
	//has been 15 minutes passed
	uint32_t currentTime = GetMilSecTick();
	if(DeviceManager_IsUpdatePending() == false && Timer_CalcTimeDiff(s_device.lastTime, currentTime) >= DEVICE_AUTO_UPDATE_INTERVAL){
		//send update frame to device
		USART_FRAME frame = USART_GenerateDeviceReport();
		USART_SendFrame(frame);
		DeviceManager_SetUpdatePending();
	}

	//wait for update (1 minute after send update frame)
	if(DeviceManager_IsUpdatePending() &&
			Timer_CalcTimeDiff(s_device.lastTime, currentTime) >= DEVICE_AUTO_UPDATE_INTERVAL + DEVICE_UPDATE_PENDING_WAIT_INTERVAL){
		//set device as offline
		DeviceManager_SetDeviceOffline();
		DeviceManager_ClearUpdatePending();

		//call app layer to handler connection state changed
		if(s_deviceHandler != NULL){
			s_deviceHandler();
		}
	}
}



static bool DeviceManager_IsUpdatePending(void){
	return s_device.updatePending;
}
static void DeviceManager_SetUpdatePending(void){
	s_device.updatePending = true;
}
static void DeviceManager_ClearUpdatePending(void){
	s_device.updatePending = false;
}

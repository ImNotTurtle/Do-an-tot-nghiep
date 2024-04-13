/*
 * DeviceManager.h
 *
 *  Created on: Mar 16, 2024
 *      Author: Administrator
 */

#ifndef DEVICEMANAGER_H_
#define DEVICEMANAGER_H_
#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "USART.h"
#include "Utils.h"


#define DEVICE_AUTO_UPDATE_INTERVAL					(15 * 60 * 1000) // 15 minutes
#define DEVICE_UPDATE_PENDING_WAIT_INTERVAL			(1 * 60 * 1000) // 1 minute

typedef enum{
	ONLINE,
	OFFLINE,
	UNKNOWN = 0xFF
}CONNECTION_STATE_e;

typedef enum{
	REM,
	UNQUALIFIED = 0xFF
}DEVICE_TYPE_e;

typedef struct{
	CONNECTION_STATE_e state;
	DEVICE_TYPE_e type;
	uint8_t level;
	uint32_t lastTime;
	bool updatePending;
}DEVICE_s;


typedef void (*deviceManagingHandler)(void);

void DeviceManager_DeviceInit(deviceManagingHandler handler, DEVICE_TYPE_e type);
DEVICE_s DeviceManager_GetDevice(void);
void DeviceManager_SetDeviceOnline(void);
void DeviceManager_SetDeviceOffline(void);
void DeviceManager_SetDeviceLevel(uint8_t level);
void DeviceManager_UpdateLastTime(uint32_t time);
void DeviceManager_Update(void);



#endif /* DEVICEMANAGER_H_ */

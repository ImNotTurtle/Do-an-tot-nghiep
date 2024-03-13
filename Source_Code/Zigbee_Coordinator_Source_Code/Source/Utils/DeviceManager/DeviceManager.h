/*
 * DeviceManager.h
 *
 *  Created on: Feb 29, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_UTILS_DEVICEMANAGER_DEVICEMANAGER_H_
#define SOURCE_UTILS_DEVICEMANAGER_DEVICEMANAGER_H_
#include "app/framework/include/af.h"
#include "custom_token.h"
#include <string.h>
#include <stdbool.h>

#define MAX_DEVICES					10
#define DEVICE_ID_INVALID			0xFF
#define NODE_ID_INVALID				0xFFFF

Device_s DeviceManager_AddDevice(EmberNodeId nodeId, uint8_t *modelId, uint8_t length);
void DeviceManager_DeleteDevice(EmberNodeId nodeId);

Device_s DeviceManager_GetDeviceAt(uint8_t index);
Device_s* DeviceManager_GetDeviceList(void);
uint8_t DeviceManager_GetDeviceListLength(void);

DeviceType_e DeviceManager_ToDeviceType(uint8_t *modelId, uint8_t length);
DeviceType_e DeviceManager_GetDeviceType(EmberNodeId nodeId);

ConnectState_e DeviceManager_GetDeviceConnectionState(EmberNodeId nodeId);
bool DeviceManager_SetDeviceOnline(EmberNodeId nodeId);
bool DeviceManager_SetDeviceOffline(EmberNodeId nodeId);
void DeviceManager_UpdateLastTime(EmberNodeId nodeId, uint32_t time);

bool DeviceManager_IsValidDevice(Device_s device);
bool DeviceManager_IsDeviceExist(EmberNodeId nodeId);



#endif /* SOURCE_UTILS_DEVICEMANAGER_DEVICEMANAGER_H_ */

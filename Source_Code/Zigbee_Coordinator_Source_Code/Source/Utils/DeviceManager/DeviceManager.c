/*
 * DeviceManager.c
 *
 *  Created on: Feb 29, 2024
 *      Author: Administrator
 */
#include "DeviceManager.h"
//#include "custom_token.h"

#define DEVICE_INDEX_INVALID				0xFF

static DeviceManager_s s_manager;

static void DeviceManager_SetManager(void){
	halCommonSetToken(TOKEN_DEVICE_MANAGER, &s_manager);
}
static void DeviceManager_GetManager(void){
	halCommonGetToken(&s_manager, TOKEN_DEVICE_MANAGER);
}


static uint8_t GetDeviceIndex(EmberNodeId nodeId);
static Device_s GetInvalidDevice(void);

//add new device to the list, return new device id (return NODE_ID_INVALID if the device is already added)
Device_s DeviceManager_AddDevice(EmberNodeId nodeId, uint8_t *modelId, uint8_t length){
	Device_s retDevice;
	DeviceManager_GetManager();
	if(s_manager.length == 0){//first device to add, return HC id (which is 0)
		retDevice.nodeId = 0x0000;
		retDevice.type = HC;
		retDevice.state = ONLINE;
	}
	else{
		//check if the device id already in the list
		if(DeviceManager_IsDeviceExist(nodeId)){//cannot add
			return GetInvalidDevice();
		}
		//device not in the list
		DeviceType_e type = DeviceManager_ToDeviceType(modelId, length);
		retDevice.nodeId = nodeId;
		retDevice.type = type;
		retDevice.state = ONLINE;
	}

	//add device to list if the device is ready to add (valid and not currently in the list)
	if(DeviceManager_IsValidDevice(retDevice)){
		s_manager.deviceList[s_manager.length] = retDevice;
		s_manager.length++;
	}

	DeviceManager_SetManager();
	return retDevice;
}
//delete the device
void DeviceManager_DeleteDevice(EmberNodeId nodeId){
	DeviceManager_GetManager();
	uint8_t index = GetDeviceIndex(nodeId);
	if(index != DEVICE_INDEX_INVALID){
		for(uint8_t i = index; i < s_manager.length - 1; i++){
			s_manager.deviceList[i] = s_manager.deviceList[i + 1];
		}
		s_manager.length--;
	}
	DeviceManager_SetManager();
}
Device_s DeviceManager_GetDeviceAt(uint8_t index){
	DeviceManager_GetManager();
	if(index >= s_manager.length){
		return GetInvalidDevice();
	}
	return s_manager.deviceList[index];
}
Device_s* DeviceManager_GetDeviceList(void){
	DeviceManager_GetManager();
	return s_manager.deviceList;
}
uint8_t DeviceManager_GetDeviceListLength(void){
	DeviceManager_GetManager();
	return s_manager.length;
}
//translate model ID to device type
DeviceType_e DeviceManager_ToDeviceType(uint8_t *modelId, uint8_t length){
	if(modelId == NULL) return UNQUALIFIED;
	if(strncmp((char*)modelId, (char*)"HC", length) == 0){
		return HC;
	}
	if(strncmp((char*)modelId, (char*)"REM", length) == 0){
		return REM;
	}
	if(strncmp((char*)modelId, (char*)"QUAT", length) == 0){
		return QUAT;
	}
	return UNQUALIFIED;
}

DeviceType_e DeviceManager_GetDeviceType(EmberNodeId nodeId){
	DeviceManager_GetManager();
	for(uint8_t i = 0; i < s_manager.length; i++){
		if(nodeId == s_manager.deviceList[i].nodeId){
			return s_manager.deviceList[i].type;
		}
	}
	return UNQUALIFIED;
}
ConnectState_e DeviceManager_GetDeviceConnectionState(EmberNodeId nodeId){
	uint8_t index = GetDeviceIndex(nodeId);
	if(index != DEVICE_INDEX_INVALID){
		return s_manager.deviceList[index].state;
	}
	return UNKNOWN;
}
//return true if set online success (is currently in the list), false otherwise
bool DeviceManager_SetDeviceOnline(EmberNodeId nodeId){
	DeviceManager_GetManager();
	uint8_t index = GetDeviceIndex(nodeId);
	if(index == DEVICE_INDEX_INVALID) return false;
	s_manager.deviceList[index].state = ONLINE;
	DeviceManager_SetManager();
	return true;
}
//return true if set offline success (is currently in the list), false otherwise
bool DeviceManager_SetDeviceOffline(EmberNodeId nodeId){
	DeviceManager_GetManager();
	uint8_t index = GetDeviceIndex(nodeId);
	if(index == DEVICE_INDEX_INVALID) return false;
	s_manager.deviceList[index].state = OFFLINE;
	DeviceManager_SetManager();
	return true;
}
bool DeviceManager_IsValidDevice(Device_s device){
	return device.nodeId != NODE_ID_INVALID;
}
bool DeviceManager_IsDeviceExist(EmberNodeId nodeId){
	DeviceManager_GetManager();
	for(uint8_t i = 0; i < s_manager.length; i++){
		if(s_manager.deviceList[i].nodeId == nodeId){
			return true;
		}
	}
	return false;
}
void DeviceManager_UpdateLastTime(EmberNodeId nodeId, uint32_t time){
	DeviceManager_GetManager();
	uint8_t index = GetDeviceIndex(nodeId);
	if(index != DEVICE_INDEX_INVALID){
		s_manager.deviceList[index].lastTime = time;
	}
}

//return DEVICE_INDEX_INVALID if the device not found
static uint8_t GetDeviceIndex(EmberNodeId nodeId){
	DeviceManager_GetManager();
	for(uint8_t i = 0; i < s_manager.length; i++){
		if(nodeId == s_manager.deviceList[i].nodeId){
			return i;
		}
	}
	return DEVICE_INDEX_INVALID;
}

static Device_s GetInvalidDevice(void){
	Device_s retDevice;
	retDevice.type = UNQUALIFIED;
	retDevice.nodeId = NODE_ID_INVALID;
	retDevice.state = OFFLINE;
	return retDevice;
}

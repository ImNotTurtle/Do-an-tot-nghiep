/*
 * CurtainControl.c
 *
 *  Created on: Mar 23, 2024
 *      Author: Administrator
 */

#include "CurtainControl.h"

static curtainHandle curtainHandler = NULL;

void CurtainControl_Init(curtainHandle handle){
	curtainHandler = handle;
}

uint8_t CurtainControl_GetCurrentLevel(void){
	uint8_t level = 0;
	emberAfReadServerAttribute(0x01, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, &level, 1);
	return level;
}
void CurtainControl_Close(void){
	CurtainControl_SetLevel(LEVEL_SET, 0);
}
void CurtainControl_Open(void){
	CurtainControl_SetLevel(LEVEL_SET, 100);
}
void CurtainControl_IncreaseLevel(void){
	uint8_t level = CurtainControl_GetCurrentLevel();
	level += LEVEL_CONTROL_INCREASE_LEVEL;
	CurtainControl_SetLevel(LEVEL_INCREASE, level);
}
void CurtainControl_DecreaseLevel(void){
	uint8_t level = CurtainControl_GetCurrentLevel();
	level -= LEVEL_CONTROL_DECREASE_LEVEL;
	CurtainControl_SetLevel(LEVEL_DECREASE, level);
}
void CurtainControl_SetLevel(LEVEL_CONTROL_CMD_e levelCmd, uint8_t level){
	if(!(level >= 0 && level <= 100)){ // invalid level
		return;
	}
	emberAfCorePrintln("Current level: %d", level);
	emberAfWriteServerAttribute(0x01,
										ZCL_LEVEL_CONTROL_CLUSTER_ID,
										ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
										(uint8_t*)&level,
										ZCL_INT8U_ATTRIBUTE_TYPE);

	if(curtainHandler != NULL){
		curtainHandler(level);
	}
}
void CurtainControl_Update(void){
	uint8_t level = CurtainControl_GetCurrentLevel();
	CurtainControl_SetLevel(LEVEL_SET, level);
}

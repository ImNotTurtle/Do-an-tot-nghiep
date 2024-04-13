/*
 * CurtainControl.h
 *
 *  Created on: Mar 23, 2024
 *      Author: Administrator
 */

#ifndef SOURCE_APP_CURTAINCONTROL_CURTAINCONTROL_H_
#define SOURCE_APP_CURTAINCONTROL_CURTAINCONTROL_H_
#include "app/framework/include/af.h"


#define LEVEL_CONTROL_INCREASE_LEVEL			25
#define LEVEL_CONTROL_DECREASE_LEVEL			25


typedef enum{
	LEVEL_SET,
	LEVEL_INCREASE,
	LEVEL_DECREASE,
	LEVEL_NONE = 0xFF
}LEVEL_CONTROL_CMD_e;

typedef void (*curtainHandle)(uint8_t level);

void CurtainControl_Init(curtainHandle handle);
uint8_t CurtainControl_GetCurrentLevel(void);
void CurtainControl_Close(void);
void CurtainControl_Open(void);
void CurtainControl_SetLevel(LEVEL_CONTROL_CMD_e levelCmd, uint8_t level);
void CurtainControl_Update(void);
void CurtainControl_IncreaseLevel(void);
void CurtainControl_DecreaseLevel(void);

#endif /* SOURCE_APP_CURTAINCONTROL_CURTAINCONTROL_H_ */

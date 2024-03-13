/*
 * Button.h
 *
 *  Created on: Sep 5, 2023
 *      Author: Administrator
 */

#ifndef SOURCE_MID_BUTTON_BUTTON_H_
#define SOURCE_MID_BUTTON_BUTTON_H_

#include "app/framework/include/af.h"

#include "Source/Mid/Timer/Timer.h"

#define BUTTON_COUNT					(2U)
#define BUTTON_PRESS					(0U)
#define GPIO_DOUT						(0U)
#define BUTTON_1_PORT					(gpioPortD)
#define BUTTON_1_PIN					(4U)
#define BUTTON_2_PORT					(gpioPortD)
#define BUTTON_2_PIN					(3U)
#define BUTTON_INIT						{{BUTTON_1_PORT, BUTTON_1_PIN}, \
										{BUTTON_2_PORT, BUTTON_2_PIN}}
#define BUTTON_INVALID_INDEX			0xFF

#define HOLD_BUTTON_INTERVAL			500
#define HOLD_BUTTON_CYCLE_PER_SECOND	(1000 / HOLD_BUTTON_INTERVAL)
#define BUTTON_THRESHOLD_INTERVAL		500
#define SCAN_BUTTON_INTERVAL			100
#define BUTTON_INTERFERENCE_INTERVAL	30



typedef enum{
	press_1 = 1,
	press_2,
	press_3,
	press_4,
	press_5,
	press_6_or_more
}PRESS_EVENT_e;

typedef enum{
	hold,
	hold_1s,
	hold_2s,
	hold_3s,
	hold_4s,
	hold_5s,
	hold_6s_or_more
}HOLD_EVENT_e;

typedef struct{
	GPIO_Port_TypeDef port;
	uint8_t pin;
	uint32_t pressTime;
	uint32_t releaseTime;
	uint8_t pressCount;
	uint8_t holdCount;
}BUTTON_t;

typedef void (*buttoncallbackFunction) (uint8_t);

void ButtonControl_Init(buttoncallbackFunction, buttoncallbackFunction);
PRESS_EVENT_e ButtonControl_GetPressEvent(uint8_t index);
HOLD_EVENT_e ButtonControl_GetHoldEvent(uint8_t index);



#endif /* SOURCE_MID_BUTTON_BUTTON_H_ */

/*
 * Led.h
 *
 *  Created on: Jul 31, 2023
 *      Author: Administrator
 */

#ifndef SOURCE_MID_LED_LED_H_
#define SOURCE_MID_LED_LED_H_
#include "app/framework/include/af.h"
#include "em_timer.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include <stdbool.h>

#define LED_RGB_COUNT						2
#define LED_RGB_ELEMENT						3

#define LED_RED_1_PORT						gpioPortA
#define LED_RED_1_PIN						(4U)
#define LED_BLUE_1_PORT						gpioPortA
#define LED_BLUE_1_PIN						(0U)
#define LED_GREEN_1_PORT					gpioPortA
#define LED_GREEN_1_PIN						(3U)

#define LED_RED_2_PORT						gpioPortD
#define LED_RED_2_PIN						(0U)
#define LED_BLUE_2_PORT						gpioPortD
#define LED_BLUE_2_PIN						(2U)
#define LED_GREEN_2_PORT					gpioPortD
#define LED_GREEN_2_PIN						(1U)


#define LED_RGB_1							\
{											\
	{LED_BLUE_1_PORT, LED_BLUE_1_PIN},		\
	{LED_RED_1_PORT, LED_RED_1_PIN},		\
	{LED_GREEN_1_PORT, LED_GREEN_1_PIN},	\
}

#define LED_RGB_2							\
{											\
	{LED_BLUE_2_PORT, LED_BLUE_2_PIN},		\
	{LED_RED_2_PORT, LED_RED_2_PIN},		\
	{LED_GREEN_2_PORT, LED_GREEN_2_PIN},	\
}


#define TIMER_DIM_INIT						\
{											\
	false,									\
	false,									\
	timerPrescale1,							\
	timerClkSelHFPerClk,					\
	false,									\
	false,									\
	timerInputActionNone,					\
	timerInputActionNone,					\
	timerModeUp,							\
	false,									\
	false,									\
	false,									\
	false									\
}

#define LED_RGB_DIM_T						100


typedef enum{
	NONE,
	LED_TOGGLE,
	LED_DIM
}LED_BLINK_MODE_e;

typedef enum{
	OFF = 0x000,
	BLUE = 1U << 0,
	RED = 1U << 1,
	GREEN = 1U << 2,
	PINK = RED | BLUE,
	YELLOW = RED | GREEN
}LED_COLOR_e;

typedef enum{
	LED_1,
	LED_2
}LED_NUMBER_e;

typedef struct{
	GPIO_Port_TypeDef port;
	uint8_t pin;
}LedStruct;

typedef struct{
	bool dimHigh;
	uint32_t dimLevel;
}LED_DIM_s;

typedef struct{
	uint32_t onTime;
	uint32_t offTime;
	uint32_t blinkTime;
	LED_DIM_s ledDim;
}LED_ACT_s;

typedef struct{
	LED_BLINK_MODE_e ledBlinkMode;
	LED_COLOR_e color;
	LED_ACT_s ledAct;
}LedAction;



void LedControl_Init(void);
void LedControl_On(LED_NUMBER_e ledIndex, LED_COLOR_e color);
void LedControl_Off(LED_NUMBER_e ledIndex);
void LedControl_Toggle(LED_NUMBER_e ledIndex, LED_COLOR_e color, uint8_t blinkCount,
						uint32_t onTimeMS, uint32_t offTimeMS);


#endif /* SOURCE_MID_LED_LED_H_ */

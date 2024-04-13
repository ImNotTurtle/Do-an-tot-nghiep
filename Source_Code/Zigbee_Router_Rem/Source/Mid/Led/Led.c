/*
 * Led.c
 *
 *  Created on: Jul 31, 2023
 *      Author: Administrator
 */

#include "Led.h"


LedStruct ledRgbArray[LED_RGB_COUNT][LED_RGB_ELEMENT] = {LED_RGB_1, LED_RGB_2};
LedAction ledAction[LED_RGB_COUNT];
EmberEventControl led1ToggleEventControl, led2ToggleEventControl;
EmberEventControl *ledToggleEventControl[LED_RGB_COUNT];

static void toggleLed(LED_NUMBER_e ledIndex);

void LedControl_Init(void){
	CMU_ClockEnable(cmuClock_GPIO, true);

	for(int i = 1; i < LED_RGB_COUNT; i++){
		for(int j = 0; j < LED_RGB_ELEMENT; j++){
			GPIO_PinModeSet(ledRgbArray[i][j].port,
							ledRgbArray[i][j].pin,
							gpioModeInputPull,
							0);
		}
	}
//	ledToggleEventControl[LED_1] = (EmberEventControl*)(&led1ToggleEventControl);
	ledToggleEventControl[LED_2] = (EmberEventControl*)(&led2ToggleEventControl);
}

void LedControl_On(LED_NUMBER_e ledIndex, LED_COLOR_e color){
	for(int i = 0; i < LED_RGB_ELEMENT; i++){
		if( ((color >> i) & 0x01) == 1){
			GPIO_PinOutClear(ledRgbArray[ledIndex][i].port, ledRgbArray[ledIndex][i].pin);
		}
		else{
			GPIO_PinOutSet(ledRgbArray[ledIndex][i].port, ledRgbArray[ledIndex][i].pin);
		}
	}
}
void LedControl_Off(LED_NUMBER_e ledIndex){
	for(int i = 0; i < LED_RGB_ELEMENT; i++){
		GPIO_PinOutSet(ledRgbArray[ledIndex][i].port, ledRgbArray[ledIndex][i].pin);
	}
}

void LedControl_Toggle(LED_NUMBER_e ledIndex, LED_COLOR_e color, uint8_t blinkCount,
						uint32_t onTimeMS, uint32_t offTimeMS){
	ledAction[ledIndex].color = color;
	ledAction[ledIndex].ledAct.blinkTime = blinkCount * 2;
	ledAction[ledIndex].ledAct.onTime = onTimeMS;
	ledAction[ledIndex].ledAct.offTime = offTimeMS;
	emberEventControlSetActive(*ledToggleEventControl[ledIndex]);
}

void led1ToggleEventHandler(void){
	toggleLed(LED_1);
}

void led2ToggleEventHandler(void){
	toggleLed(LED_2);
}

static void toggleLed(LED_NUMBER_e ledIndex){
	emberEventControlSetInactive(*ledToggleEventControl[ledIndex]);

	uint8_t delayMS;
	if(ledAction[ledIndex].ledAct.blinkTime % 2 == 0){//toggle on
		LedControl_On(ledIndex, ledAction[ledIndex].color);
		delayMS = ledAction[ledIndex].ledAct.onTime;
	}
	else{//toggle off
		LedControl_Off(ledIndex);
		delayMS = ledAction[ledIndex].ledAct.offTime;
	}
	ledAction[ledIndex].ledAct.blinkTime--;
	if(ledAction[ledIndex].ledAct.blinkTime > 0){
		emberEventControlSetDelayMS(*ledToggleEventControl[ledIndex], delayMS);
	}
}






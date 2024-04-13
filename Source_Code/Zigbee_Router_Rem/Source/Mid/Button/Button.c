/*
 * Button.c
 *
 *  Created on: Sep 5, 2023
 *      Author: Administrator
 */
#include "app/framework/include/af.h"
#include "Button.h"

EmberEventControl scanButtonEventControl;
static buttoncallbackFunction s_pressCallback = NULL;
static buttoncallbackFunction s_holdCallback = NULL;
static BUTTON_t s_buttonArr[BUTTON_COUNT] = BUTTON_INIT;

static uint8_t GetButtonIndex(uint8_t pin);
static boolean IsButtonPressed(uint8_t index);



void ButtonControl_Init(buttoncallbackFunction pressCallback, buttoncallbackFunction holdCallback){
	for(uint8_t i = 0; i < BUTTON_COUNT; i++){
		GPIO_PinModeSet(s_buttonArr[i].port, s_buttonArr[i].pin, gpioModeInput, GPIO_DOUT);
	}
	//callback register
	s_pressCallback = pressCallback;
	s_holdCallback = holdCallback;

	emberEventControlSetDelayMS(scanButtonEventControl, SCAN_BUTTON_INTERVAL);
}

void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state){
	//chuyen doi tu` pin number sang index
	uint8_t index = GetButtonIndex(button);
	if(index == 0xFF) return; //invalid index

	if(state == BUTTON_PRESSED){
		s_buttonArr[index].pressCount++;
		s_buttonArr[index].pressTime = Timer_GetCounter();
	}
	else{
		s_buttonArr[index].releaseTime = Timer_GetCounter();
		//chong nhieu
		if(Timer_CalcTimeDiff(s_buttonArr[index].pressTime, s_buttonArr[index].releaseTime) <= 30 && s_buttonArr[index].pressCount > 0){
			s_buttonArr[index].pressCount--;
		}
	}
}

//scan button moi~ SCAN_BUTTON_INTERVAL ms
void scanButtonEventHandler(void){
	emberEventControlSetInactive(scanButtonEventControl);

	for(uint8_t i = 0; i < BUTTON_COUNT; i++){
		uint32_t timeCurrent = Timer_GetCounter();
		uint32_t timeDiff = Timer_CalcTimeDiff(s_buttonArr[i].pressTime, timeCurrent);
		uint32_t releaseTimeDiff = Timer_CalcTimeDiff(s_buttonArr[i].releaseTime, timeCurrent);
		if(IsButtonPressed(i)){
			//neu thoi gian giu nut >= 500ms va thoi gian giu nut chia het cho 500ms
			//(kich hoat su kien moi 500ms, sai so chenh lech la SCAN_BUTTON_INTERVAL ms)
			if(timeDiff >= BUTTON_THRESHOLD_INTERVAL){
				//update thoi gian nhan nut
				s_buttonArr[i].pressTime = Timer_GetCounter();
				s_buttonArr[i].holdCount++;
				s_buttonArr[i].pressCount = 0;
				s_holdCallback(i);
			}
		}
		else{
			if(s_buttonArr[i].pressCount > 0){
				//neu thoi gian nhan nut >= 500ms
				if(releaseTimeDiff >= BUTTON_THRESHOLD_INTERVAL){
					s_pressCallback(i);
					s_buttonArr[i].pressCount = 0;
				}
			}
		}
	}

	emberEventControlSetDelayMS(scanButtonEventControl, SCAN_BUTTON_INTERVAL);
}




boolean IsButtonPressed(uint8_t index){
	return GPIO_PinInGet(s_buttonArr[index].port, s_buttonArr[index].pin) == BUTTON_PRESS;
}
uint8_t GetButtonIndex(uint8_t pin){
	for(uint8_t i = 0; i < BUTTON_COUNT; i++){
		if(s_buttonArr[i].pin == pin) return i;
	}
	return 0xFF;
}
PRESS_EVENT_e ButtonControl_GetPressEvent(uint8_t index){
	return s_buttonArr[index].pressCount;
}
HOLD_EVENT_e ButtonControl_GetHoldEvent(uint8_t index){
	return s_buttonArr[index].holdCount / 2;
}

